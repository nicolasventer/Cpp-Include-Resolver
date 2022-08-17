#pragma once

struct A
{
	int i;

	A& operator+=(const A& rhs);
};