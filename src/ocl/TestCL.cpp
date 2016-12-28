/*
 * Copyright(C) 2016, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <AlloyContext.h>
#include <AlloyMath.h>
#include <AlloyVector.h>
#include "ocl/ComputeCL.h"
#include "ocl/ProgramCL.h"
#include "ocl/BufferCL.h"
#include "ocl/FunctionCL.h"
#include "ocl/ocl_runtime_error.h"
#include <iostream>
using namespace aly;
void SANITY_CHECK_OPENCL(){
	CLInitialize();
	std::shared_ptr<ProgramCL> prog=CLCompile("alloy_math.cl");
	std::cout<<*prog<<std::endl;
	Vector4f vecInput(16);
	vecInput.set(float4(1,2,3,1));
	float4x4 mat=MakeRotationY((float)(M_PI*0.5f))*MakeTranslation(float3(1,1,1));
	std::cout<<"Transform=\n"<<mat<<std::endl;
	BufferCL input;
	std::cout<<"Input=\n"<<vecInput<<std::endl;
	input.write(vecInput);
	for(float4& v:vecInput){
		v=mat*v;
	}
	std::cout<<"CPU Result=\n"<<vecInput<<std::endl;
	prog->function("TransformV4f")->set("vector",input).set("pose",mat).execute1D(vecInput.size());
	input.read(vecInput);
	std::cout<<"GPU Result=\n"<<vecInput<<std::endl;
	CLDestroy();
}


