#include "A.h"
#include "B.h"

A& A::operator+=(const A& rhs)
{
	i += rhs.i;
	return *this;
}
