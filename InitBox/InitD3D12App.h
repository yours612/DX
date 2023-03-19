#pragma once

#include "D3DApp.h"

class BoxApp : public D3DApp {
public:

	BoxApp();
	~BoxApp();

	virtual bool Init(HINSTANCE hInstance, int nShowCmd) override;

private:

	virtual void Draw() override;  //override用于直接明了的告诉编译器该函数用于重载父类的某个虚函数

};
