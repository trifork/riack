#include "stdafx.h"
#include "RiackHelper.h"

RiackHelper::RiackHelper(void)
{
	riack_init();
}

RiackHelper::~RiackHelper(void)
{
	riack_cleanup();
}

