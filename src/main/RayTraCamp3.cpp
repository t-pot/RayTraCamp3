// RayTraCamp3.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008 
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt 
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2 

#include <iostream>
#include <thread>
#include <exception>
#include <mutex>
#include <chrono>
#include "save.h"
#include "../renderer/renderer.h"


unsigned char *g_image = nullptr;
bool g_is_finished = false;
std::mutex mtx;

void ray_tracing(int w, int h, unsigned char *dst)
{
	renderer *r = new renderer();
	r->run(w, h, dst, &g_is_finished);

	delete r;
	g_is_finished = true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	const auto startTime = std::chrono::system_clock::now();	// 計測用時間取得

	const int rendering_time_max = 15 * 60;// 終えなくてはならない時間
	const int snap_interval = 30;// 画像をとる時間

	int w = 1024, h = 768;// 解像度

	g_image = new unsigned char[3 * w*h];

	try {
		std::thread t1(ray_tracing, w, h, g_image);

		unsigned int frames = 0; // ファイル名用フレームカウンター
		do{
			std::this_thread::sleep_for(std::chrono::seconds(snap_interval));

			char buf[256];
			sprintf_s(buf, "%d.bmp", ++frames);
			mtx.lock();
			SaveImage(buf, g_image, w, h);
			mtx.unlock();

			const auto endTime = std::chrono::system_clock::now();
			const auto timeSpan = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

			// 次の時間更新までの時間が足りないので終わる
			if (rendering_time_max - snap_interval < timeSpan){
				g_is_finished = true;
			}

		} while (!g_is_finished);

		t1.join();// 終わっていないようなら終わらせる
	}
	catch (std::exception &ex) {
		std::cerr << ex.what() << std::endl;
	}

	delete[] g_image;

	return 0;
}

