//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// LOOKAT-1.0: Good old application entry point! This is where it all begins.
// WinMain() is like a main() function.
// This will construct a DXProceduralProject object and pass that to Win32Application.

#include "stdafx.h"
#include "DXProceduralProject.h"
#include "Creature.h"

#define CPU_CODE_COMPLETE 1

void generate() {
	// Needs updating with DXR buffers and controls
	/*let bigEmptyArray : Array<number> = new Array(100);
	bigEmptyArray.fill(0);
	raymarchShader.setSpineLocations(bigEmptyArray);
	raymarchShader.setSpineRadii(bigEmptyArray);
	raymarchShader.setHead(bigEmptyArray);
	raymarchShader.setAppenData(bigEmptyArray);
	raymarchShader.setJointLocations(bigEmptyArray);
	raymarchShader.setLimbLengths(bigEmptyArray);
	raymarchShader.setJointRadii(bigEmptyArray);
	raymarchShader.setRotations([]); // mat4's
	raymarchShader.setAppenData(bigEmptyArray);
	raymarchShader.setAppenRad(bigEmptyArray);
	raymarchShader.setAppenRotations([]) // mat4's
	//bigEmptyArray = new Array(100);
	// bigEmptyArray.fill(1);
	// raymarchShader.setAppenBools(bigEmptyArray);*/

	/*switch (controls.headType) {
	case 'random': headType = -1;
		break;
	case 'BUGG': headType = 0;
		break;
	case 'DINO': headType = 1;
		break;
	case 'TROLLE': headType = 2;
		break;
	}
	creature.generate(textures.length, controls.numLimbSets, headType);*/

	/*let appenBools : Array<number> = []; // 0 will be foot, 1 will be hand
	let armsNow : boolean = false;
	for (let i : number = 0; i < creature.limbs.length; i++) {
		if (armsNow) {
			appenBools.push(1);
			continue;
		}
		if (creature.limbs[i].isLeg) {
			appenBools.push(0);
		}
		else {
			armsNow = true;
			appenBools.push(1);
		}
	}
	raymarchShader.setAppenBools(appenBools);*/
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    DXProceduralProject project(1280, 720, L"DXR Project - Procedural Geometry");
#if CPU_CODE_COMPLETE
    return Win32Application::Run(&project, hInstance, nCmdShow);
#endif
}
