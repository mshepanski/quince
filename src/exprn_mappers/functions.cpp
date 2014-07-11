/*
    Copyright 2014 Michael Shepanski

    This file is part of the quince library.

    Quince is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Quince is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with quince.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <quince/mappers/optional_mapper.h>
#include <quince/exprn_mappers/cast.h>
#include <quince/exprn_mappers/constant.h>
#include <quince/exprn_mappers/detail/exprn_mapper.h>
#include <quince/exprn_mappers/functions.h>
#include <quince/exprn_mappers/operators.h>
#include <quince/query.h>

using boost::optional;
using std::string;


namespace quince {

namespace {
    QUINCE_DEFINE_SERVER_ONLY_TYPE(asterisk);  // pseudo-type for the "*" in "COUNT(*)"

    QUINCE_DEFINE_SERVER_ONLY_TYPE(any_sum);  // what the server makes when it sums stuff.  It may or may not be some normal type, but it depends on arg and dbms, so I pretend it's not normal.
	QUINCE_DEFINE_SERVER_ONLY_TYPE(any_avg);  // what the server makes when it averages stuff.  In fact it's some normal type, but it depends on arg and dbms, so I pretend it's not normal.

	template<typename T>
	exprn_mapper<optional<double>>
	generic_avg(const abstract_mapper<T> &arg) {
		return cast<optional<double>>(function<any_avg>("avg", arg));
	}

    template<typename Return, typename Arg>
    exprn_mapper<Return>
    generic_sum(const abstract_mapper<Arg> &arg) {
        return cast<optional<Return>>(function<any_sum>("sum", arg)).get_value_or(0);
    }
}

const exprn_mapper<int64_t> count_all = count(constant<asterisk>("*"));
const predicate &empty = count_all == int64_t(0);

exprn_mapper<int16_t>   abs(const abstract_mapper<int16_t> &arg)    { return function<int16_t>("abs", arg); }
exprn_mapper<int32_t>   abs(const abstract_mapper<int32_t> &arg)    { return function<int32_t>("abs", arg); }
exprn_mapper<int64_t>   abs(const abstract_mapper<int64_t> &arg)    { return function<int64_t>("abs", arg); }
exprn_mapper<float>     abs(const abstract_mapper<float> &arg)      { return function<float>("abs", arg); }
exprn_mapper<double>    abs(const abstract_mapper<double> &arg)     { return function<double>("abs", arg); }

exprn_mapper<optional<double>>  avg(const abstract_mapper<int16_t> &arg)    { return generic_avg(arg); }
exprn_mapper<optional<double>>  avg(const abstract_mapper<int32_t> &arg)    { return generic_avg(arg); }
exprn_mapper<optional<double>>  avg(const abstract_mapper<int64_t> &arg)    { return generic_avg(arg); }
exprn_mapper<optional<double>>  avg(const abstract_mapper<float> &arg)      { return generic_avg(arg); }
exprn_mapper<optional<double>>  avg(const abstract_mapper<double> &arg)     { return generic_avg(arg); }

exprn_mapper<int64_t>   count(const abstract_mapper_base &arg)      { return function<int64_t>("count", arg ); }

exprn_mapper<int64_t>   sum(const abstract_mapper<int16_t> &arg)    { return generic_sum<int64_t>(arg); }
exprn_mapper<int64_t>   sum(const abstract_mapper<int32_t> &arg)    { return generic_sum<int64_t>(arg); }
exprn_mapper<int64_t>   sum(const abstract_mapper<int64_t> &arg)    { return generic_sum<int64_t>(arg); }
exprn_mapper<double>    sum(const abstract_mapper<float> &arg)      { return generic_sum<double>(arg); }
exprn_mapper<double>    sum(const abstract_mapper<double> &arg)     { return generic_sum<double>(arg); }

exprn_mapper<string>    lower(const abstract_mapper<string> &arg)   { return function<string>("lower", arg); }
exprn_mapper<string>    upper(const abstract_mapper<string> &arg)   { return function<string>("upper", arg); }
exprn_mapper<int32_t>   length(const abstract_mapper<string> &arg)  { return function<int32_t>("length", arg); }

}
