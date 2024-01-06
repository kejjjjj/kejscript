#include "datatype.hpp"

void datatype::implicit_cast(datatype* other)
{
	datatype_e this_type = type();
	datatype_e other_type = other->type();

	datatype* stronger = 0;
	datatype* weaker = 0;

	if (this_type > other_type) {
		stronger = this;
		weaker = other;
	}
	else if (this_type < other_type) {
		stronger = other;
		weaker = this;
	}
	else
		return;

	switch (stronger->type()) {
	case datatype_e::int_t:
		*weaker = datatype::create_type_copy<integer_dt, int>(datatype::cast_normal<integer_dt>(weaker));
		break;
	case datatype_e::double_t:
		*weaker = datatype::create_type_copy<double_dt, double>(datatype::cast_normal<double_dt>(weaker));
		break;
	}


}