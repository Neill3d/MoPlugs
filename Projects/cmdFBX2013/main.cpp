
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: main.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IO\CmdFBX.h"
#include "IO\FbxUtils.h"
#include <stdio.h>
#include <memory>

int _tmain(int argc, _TCHAR* argv[])
{
	std::auto_ptr<InputModelData> data(new InputModelData());

	if (argc > 2)
	{
		if (CmdMakeSnapshotFBX_Receive(data.get()) == true)
		{
			MakeSnapshotFBX( argv[1], argv[2], *data, false );
		}
	}

	return 0;
}

