#pragma once

typedef double (*InitFn)(double w, double h);
typedef double (*MainLoopFn)(double w, double h, double tick, double mx, double my, double dmx, double dmy, double dw, double lmb);

bool LoadDLL(InitFn& init, MainLoopFn& ml, const char* dllname = "/rlml.dll");
void UnloadDLL();