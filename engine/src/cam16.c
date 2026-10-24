/* SPDX-License-Identifier: GPL-3.0-only */
#include "cam16.h"

#include <math.h>
#include <threads.h>

#include "seurat/color.h"

#define PI 3.14159265358979323846

static const double white_point[3] = {95.047, 100.0, 108.883};

static const double xyz_to_cam[3][3] = {
    {0.401288, 0.650173, -0.051461},
    {-0.250268, 1.204414, 0.045854},
    {-0.002079, 0.048952, 0.953127},
};

static const double cam_to_xyz[3][3] = {
    {1.86206786, -1.01125463, 0.14918677},
    {0.38752654, 0.62144744, -0.00897398},
    {-0.01584150, -0.03412294, 1.04996444},
};

static struct seurat_viewing_conditions default_vc;
static once_flag default_vc_once = ONCE_FLAG_INIT;

static double signum(double x)
{
    return (x > 0.0) - (x < 0.0);
}

static double lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

static void init_default_vc(void)
{
    struct seurat_viewing_conditions *vc = &default_vc;
    double adapting_luminance = 200.0 / PI * seurat_y_from_lstar(50.0) / 100.0;
    double surround = 2.0;
    double f = 0.8 + surround / 10.0;
    double rgb_w[3];
    double rgb_a[3];
    double d;
    double k;
    double k4;
    double k4f;

    for (int i = 0; i < 3; i++)
        rgb_w[i] = xyz_to_cam[i][0] * white_point[0] +
                   xyz_to_cam[i][1] * white_point[1] +
                   xyz_to_cam[i][2] * white_point[2];

    vc->c = f >= 0.9 ? lerp(0.59, 0.69, (f - 0.9) * 10.0)
                     : lerp(0.525, 0.59, (f - 0.8) * 10.0);
    vc->nc = f;

    d = f * (1.0 - (1.0 / 3.6) * exp((-adapting_luminance - 42.0) / 92.0));
    if (d < 0.0)
        d = 0.0;
    if (d > 1.0)
        d = 1.0;

    for (int i = 0; i < 3; i++)
        vc->rgb_d[i] = d * (100.0 / rgb_w[i]) + 1.0 - d;

    k = 1.0 / (5.0 * adapting_luminance + 1.0);
    k4 = k * k * k * k;
    k4f = 1.0 - k4;
    vc->fl = k4 * adapting_luminance +
             0.1 * k4f * k4f * cbrt(5.0 * adapting_luminance);
    vc->fl_root = pow(vc->fl, 0.25);

    vc->n = seurat_y_from_lstar(50.0) / white_point[1];
    vc->z = 1.48 + sqrt(vc->n);
    vc->nbb = 0.725 / pow(vc->n, 0.2);
    vc->ncb = vc->nbb;

    for (int i = 0; i < 3; i++) {
        double x = pow(vc->fl * vc->rgb_d[i] * rgb_w[i] / 100.0, 0.42);

        rgb_a[i] = 400.0 * x / (x + 27.13);
    }
    vc->aw = (40.0 * rgb_a[0] + 20.0 * rgb_a[1] + rgb_a[2]) / 20.0 * vc->nbb;
}

const struct seurat_viewing_conditions *seurat_viewing_conditions_default(void)
{
    call_once(&default_vc_once, init_default_vc);
    return &default_vc;
}

static double q_from_j(const struct seurat_viewing_conditions *vc, double j)
{
    return 4.0 / vc->c * sqrt(j / 100.0) * (vc->aw + 4.0) * vc->fl_root;
}

struct seurat_cam16 seurat_cam16_from_argb(uint32_t argb)
{
    const struct seurat_viewing_conditions *vc =
        seurat_viewing_conditions_default();
    struct seurat_cam16 cam;
    double xyz[3];
    double rgb_a[3];
    double a, b, u, p2, hue, hue_prime, e_hue, p1, t, alpha, ac;

    seurat_xyz_from_argb(argb, xyz);

    for (int i = 0; i < 3; i++) {
        double c = xyz_to_cam[i][0] * xyz[0] + xyz_to_cam[i][1] * xyz[1] +
                   xyz_to_cam[i][2] * xyz[2];
        double d = vc->rgb_d[i] * c;
        double af = pow(vc->fl * fabs(d) / 100.0, 0.42);

        rgb_a[i] = signum(d) * 400.0 * af / (af + 27.13);
    }

    a = (11.0 * rgb_a[0] - 12.0 * rgb_a[1] + rgb_a[2]) / 11.0;
    b = (rgb_a[0] + rgb_a[1] - 2.0 * rgb_a[2]) / 9.0;
    u = (20.0 * rgb_a[0] + 20.0 * rgb_a[1] + 21.0 * rgb_a[2]) / 20.0;
    p2 = (40.0 * rgb_a[0] + 20.0 * rgb_a[1] + rgb_a[2]) / 20.0;

    hue = atan2(b, a) * 180.0 / PI;
    if (hue < 0.0)
        hue += 360.0;
    else if (hue >= 360.0)
        hue -= 360.0;

    ac = p2 * vc->nbb;
    cam.j = 100.0 * pow(ac / vc->aw, vc->c * vc->z);
    cam.q = q_from_j(vc, cam.j);

    hue_prime = hue < 20.14 ? hue + 360.0 : hue;
    e_hue = 0.25 * (cos(hue_prime * PI / 180.0 + 2.0) + 3.8);
    p1 = 50000.0 / 13.0 * e_hue * vc->nc * vc->ncb;
    t = p1 * hypot(a, b) / (u + 0.305);
    alpha = pow(t, 0.9) * pow(1.64 - pow(0.29, vc->n), 0.73);

    cam.hue = hue;
    cam.chroma = alpha * sqrt(cam.j / 100.0);
    cam.m = cam.chroma * vc->fl_root;
    cam.s = 50.0 * sqrt(alpha * vc->c / (vc->aw + 4.0));
    return cam;
}

struct seurat_cam16 seurat_cam16_from_jch(double j, double chroma, double hue)
{
    const struct seurat_viewing_conditions *vc =
        seurat_viewing_conditions_default();
    struct seurat_cam16 cam;
    double alpha = (chroma == 0.0 || j == 0.0) ? 0.0 : chroma / sqrt(j / 100.0);

    cam.hue = hue;
    cam.chroma = chroma;
    cam.j = j;
    cam.q = q_from_j(vc, j);
    cam.m = chroma * vc->fl_root;
    cam.s = 50.0 * sqrt(alpha * vc->c / (vc->aw + 4.0));
    return cam;
}

uint32_t seurat_cam16_to_argb(const struct seurat_cam16 *cam)
{
    const struct seurat_viewing_conditions *vc =
        seurat_viewing_conditions_default();
    double alpha = (cam->chroma == 0.0 || cam->j == 0.0)
                       ? 0.0
                       : cam->chroma / sqrt(cam->j / 100.0);
    double t = pow(alpha / pow(1.64 - pow(0.29, vc->n), 0.73), 1.0 / 0.9);
    double h_rad = cam->hue * PI / 180.0;
    double e_hue = 0.25 * (cos(h_rad + 2.0) + 3.8);
    double ac = vc->aw * pow(cam->j / 100.0, 1.0 / vc->c / vc->z);
    double p1 = e_hue * (50000.0 / 13.0) * vc->nc * vc->ncb;
    double p2 = ac / vc->nbb;
    double h_sin = sin(h_rad);
    double h_cos = cos(h_rad);
    double gamma = 23.0 * (p2 + 0.305) * t /
                   (23.0 * p1 + 11.0 * t * h_cos + 108.0 * t * h_sin);
    double a = gamma * h_cos;
    double b = gamma * h_sin;
    double rgb_a[3] = {
        (460.0 * p2 + 451.0 * a + 288.0 * b) / 1403.0,
        (460.0 * p2 - 891.0 * a - 261.0 * b) / 1403.0,
        (460.0 * p2 - 220.0 * a - 6300.0 * b) / 1403.0,
    };
    double rgb_f[3];
    double xyz[3];

    for (int i = 0; i < 3; i++) {
        double abs_a = fabs(rgb_a[i]);
        double base = 27.13 * abs_a / (400.0 - abs_a);
        double c;

        if (base < 0.0)
            base = 0.0;
        c = signum(rgb_a[i]) * 100.0 / vc->fl * pow(base, 1.0 / 0.42);
        rgb_f[i] = c / vc->rgb_d[i];
    }

    for (int i = 0; i < 3; i++)
        xyz[i] = cam_to_xyz[i][0] * rgb_f[0] + cam_to_xyz[i][1] * rgb_f[1] +
                 cam_to_xyz[i][2] * rgb_f[2];

    return seurat_argb_from_xyz(xyz);
}

double seurat_cam16_distance(const struct seurat_cam16 *a,
                             const struct seurat_cam16 *b)
{
    double ja = (1.0 + 100.0 * 0.007) * a->j / (1.0 + 0.007 * a->j);
    double jb = (1.0 + 100.0 * 0.007) * b->j / (1.0 + 0.007 * b->j);
    double ma = log(1.0 + 0.0228 * a->m) / 0.0228;
    double mb = log(1.0 + 0.0228 * b->m) / 0.0228;
    double ra = a->hue * PI / 180.0;
    double rb = b->hue * PI / 180.0;
    double dj = ja - jb;
    double da = ma * cos(ra) - mb * cos(rb);
    double db = ma * sin(ra) - mb * sin(rb);
    double de = sqrt(dj * dj + da * da + db * db);

    return 1.41 * pow(de, 0.63);
}
