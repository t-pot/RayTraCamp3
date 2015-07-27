
#pragma once


#include <climits>



class Random {
	unsigned int seed_[4];
public:
	unsigned int next(void) {
		const unsigned int t = seed_[0] ^ (seed_[0] << 11);
		seed_[0] = seed_[1];
		seed_[1] = seed_[2];
		seed_[2] = seed_[3];
		return seed_[3] = (seed_[3] ^ (seed_[3] >> 19)) ^ (t ^ (t >> 8));
	}

	double next01(void) {
		return (double)next() / UINT_MAX;
	}

	Random(const unsigned int initial_seed) {
		unsigned int s = initial_seed;
		for (int i = 1; i <= 4; i++){
			seed_[i - 1] = s = 1812433253U * (s ^ (s >> 30)) + i;
		}
	}
};

struct Vec {
	double x = 0.0, y = 0.0, z = 0.0;                  // position, also color (r,g,b) 
	Vec(double x_ = 0, double y_ = 0, double z_ = 0){ x = x_; y = y_; z = z_; }
	Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
	Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
	Vec operator*(double b) const { return Vec(x*b, y*b, z*b); }
	Vec mult(const Vec &b) const { return Vec(x*b.x, y*b.y, z*b.z); }
	Vec& norm(){ return *this = *this * (1 / sqrt(x*x + y*y + z*z)); }
	double dot(const Vec &b) const { return x*b.x + y*b.y + z*b.z; } // cross: 
	Vec operator%(Vec&b){ return Vec(y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x); }
};
struct Ray { Vec o, d; Ray(Vec o_, Vec d_) : o(o_), d(d_) {} };
enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance() 
struct Sphere {
	double rad;       // radius 
	Vec p, e, c;      // position, emission, color 
	Refl_t refl;      // reflection type (DIFFuse, SPECular, REFRactive) 
	Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_) :
		rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
	double intersect(const Ray &r) const { // returns distance, 0 if nohit 
		Vec op = p - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
		double t, eps = 1e-4, b = op.dot(r.d), det = b*b - op.dot(op) + rad*rad;
		if (det<0) return 0; else det = sqrt(det);
		return (t = b - det)>eps ? t : ((t = b + det)>eps ? t : 0);
	}
};
Sphere spheres[] = {//Scene: radius, position, emission, color, material 
	Sphere(1e5, Vec(1e5 + 1, 40.8, 81.6), Vec(), Vec(.75, .25, .25), DIFF),//Left 
	Sphere(1e5, Vec(-1e5 + 99, 40.8, 81.6), Vec(), Vec(.25, .25, .75), DIFF),//Rght 
	Sphere(1e5, Vec(50, 40.8, 1e5), Vec(), Vec(.75, .75, .75), DIFF),//Back 
	Sphere(1e5, Vec(50, 40.8, -1e5 + 170), Vec(), Vec(), DIFF),//Frnt 
	Sphere(1e5, Vec(50, 1e5, 81.6), Vec(), Vec(.75, .75, .75), DIFF),//Botm 
	Sphere(1e5, Vec(50, -1e5 + 81.6, 81.6), Vec(), Vec(.75, .75, .75), DIFF),//Top 
	Sphere(16.5, Vec(27, 16.5, 47), Vec(), Vec(1, 1, 1)*.999, SPEC),//Mirr 
	Sphere(16.5, Vec(73, 16.5, 78), Vec(), Vec(1, 1, 1)*.999, REFR),//Glas 
	Sphere(600, Vec(50, 681.6 - .27, 81.6), Vec(12, 12, 12), Vec(), DIFF) //Lite 
};
inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }
inline int toInt(double x){ return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }
inline bool intersect(const Ray &r, double &t, int &id){
	double n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
	for (int i = int(n); i--;) if ((d = spheres[i].intersect(r)) && d<t){ t = d; id = i; }
	return t<inf;
}
Vec radiance(const Ray &r, int depth, Random &rnd){
	double t;                               // distance to intersection 
	int id = 0;                               // id of intersected object 
	if (!intersect(r, t, id)) return Vec(); // if miss, return black 
	const Sphere &obj = spheres[id];        // the hit object 
	Vec x = r.o + r.d*t, n = (x - obj.p).norm(), nl = n.dot(r.d)<0 ? n : n*-1, f = obj.c;
	double p = f.x>f.y && f.x>f.z ? f.x : f.y>f.z ? f.y : f.z; // max refl 
	if (++depth>5) if (rnd.next01()<p) f = f*(1 / p); else return obj.e; //R.R. 
	if (100<depth) return obj.e; //R.R. 
	if (obj.refl == DIFF){                  // Ideal DIFFUSE reflection 
		double r1 = 2 * M_PI*rnd.next01(), r2 = rnd.next01(), r2s = sqrt(r2);
		Vec w = nl, u = ((fabs(w.x)>.1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w%u;
		Vec d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2)).norm();
		return obj.e + f.mult(radiance(Ray(x, d), depth, rnd));
	}
	else if (obj.refl == SPEC)            // Ideal SPECULAR reflection 
		return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, rnd));
	Ray reflRay(x, r.d - n * 2 * n.dot(r.d));     // Ideal dielectric REFRACTION 
	bool into = n.dot(nl)>0;                // Ray from outside going in? 
	double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
	if ((cos2t = 1 - nnt*nnt*(1 - ddn*ddn))<0)    // Total internal reflection 
		return obj.e + f.mult(radiance(reflRay, depth, rnd));
	Vec tdir = (r.d*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).norm();
	double a = nt - nc, b = nt + nc, R0 = a*a / (b*b), c = 1 - (into ? -ddn : tdir.dot(n));
	double Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
	return obj.e + f.mult(depth>2 ? (rnd.next01()<P ?   // Russian roulette 
		radiance(reflRay, depth, rnd)*RP : radiance(Ray(x, tdir), depth, rnd)*TP) :
		radiance(reflRay, depth, rnd)*Re + radiance(Ray(x, tdir), depth, rnd)*Tr);
}

extern std::mutex mtx;

class renderer{
public:
	renderer(){}
	~renderer(){}
	inline void run(int w, int h, unsigned char *dst, bool *p_is_finished)
	{
		// ##################### ��������
		// global settings
		const int iteration_max = 256;
		Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm()); // cam pos, dir 
		Vec cx = Vec(w*.5135 / h), cy = (cx%cam.d).norm()*.5135;
		Vec *r = new Vec[4*w*h];// �T�u�s�N�Z���܂Ŋ܂߂��t���[���o�b�t�@
		for (int i = 0; i < 4 * w*h; i++) r[i] = Vec(0, 0, 0);
		// ##################### �����܂ł�

		for (int l = 0; l < iteration_max; l++)
		{
			// ##################### ��������
			// ray tracing!!!
			const int samps = 8; // # samples iteration_max*samps ���J�������烌�C���΂����ɂȂ�
#pragma omp parallel for schedule(dynamic, 1)       // OpenMP 
			for (int y = 0; y < h; y++){                       // Loop over image rows 
				fprintf(stderr, "\r[iteration %d]: Rendering (%d spp) %5.2f%%", l, samps * 4, 100.*y / (h - 1));
				Random rnd(y + l * w * h + 1);
				for (unsigned short x = 0; x < w; x++){   // Loop cols 
					for (int sy = 0, i = (h - y - 1)*w + x; sy < 2; sy++){     // 2x2 subpixel rows 
						for (int sx = 0; sx < 2; sx++){        // 2x2 subpixel cols 
							Vec &ri = r[4 * i + 2 * sy + sx];
							for (int s = 0; s < samps; s++){
								double r1 = 2 * rnd.next01(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
								double r2 = 2 * rnd.next01(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
								Vec d = cx*(((sx + .5 + dx) / 2 + x) / w - .5) +
										cy*(((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
								ri = ri + radiance(Ray(cam.o + d * 140, d.norm()), 0, rnd)*(1. / samps);
							} // Camera rays are pushed ^^^^^ forward to start in interior 
						}
					}
				}
			}
			// ##################### �����܂ł�Ǝ�renderer�ŕύX���܂��傤

			// ���Ԑ؂�ɂȂ��Ă���悤�Ȃ炨�ƂȂ����~�߂�
			if (*p_is_finished) break;

			// ���ʂ���ʃC���[�W�ɏ����o�� (BMP�p�ɏ㉺���])
			std::lock_guard<std::mutex> lock(mtx);
			const double coeff = 0.25 / (double)(l+1);
			int addr = 0;
			for (int y = 0; y < h; y++){ 
				const Vec *v = &r[4 * (h - 1 - y) * w];
				for (unsigned short x = 0; x < w; x++){
					Vec c = v[0] + v[1] + v[2] + v[3];// �����ŃT�u�s�N�Z���̍������s��
					dst[addr + 0] = (unsigned char)(clamp(c.x * coeff) * 255.0);
					dst[addr + 1] = (unsigned char)(clamp(c.y * coeff) * 255.0);
					dst[addr + 2] = (unsigned char)(clamp(c.z * coeff) * 255.0);
					addr += 3;
					v+=4;
				}
			}
		}
	}
};
