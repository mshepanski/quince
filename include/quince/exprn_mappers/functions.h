#ifndef QUINCE__exprn_mappers__functions_h
#define QUINCE__exprn_mappers__functions_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <stdint.h>
#include <quince/exprn_mappers/function.h>


namespace quince {

class abstract_mapper_base;
template<typename> class exprn_mapper;
typedef exprn_mapper<bool> predicate;


extern const exprn_mapper<int64_t> count_all;
extern const predicate &empty;

exprn_mapper<int16_t> abs(const abstract_mapper<int16_t> &);
exprn_mapper<int32_t> abs(const abstract_mapper<int32_t> &);
exprn_mapper<int64_t> abs(const abstract_mapper<int64_t> &);
exprn_mapper<float> abs(const abstract_mapper<float> &);
exprn_mapper<double> abs(const abstract_mapper<double> &);

exprn_mapper<std::string> lower(const abstract_mapper<std::string> &);
exprn_mapper<std::string> upper(const abstract_mapper<std::string> &);

exprn_mapper<int32_t> length(const abstract_mapper<std::string> &);

exprn_mapper<boost::optional<double>> avg(const abstract_mapper<int16_t> &);
exprn_mapper<boost::optional<double>> avg(const abstract_mapper<int32_t> &);
exprn_mapper<boost::optional<double>> avg(const abstract_mapper<int64_t> &);
exprn_mapper<boost::optional<double>> avg(const abstract_mapper<float> &);
exprn_mapper<boost::optional<double>> avg(const abstract_mapper<double> &);

exprn_mapper<int64_t> count(const abstract_mapper_base &);

exprn_mapper<int64_t> sum(const abstract_mapper<int16_t> &);
exprn_mapper<int64_t> sum(const abstract_mapper<int32_t> &);
exprn_mapper<int64_t> sum(const abstract_mapper<int64_t> &);
exprn_mapper<double> sum(const abstract_mapper<float> &);
exprn_mapper<double> sum(const abstract_mapper<double> &);


template<typename T>
exprn_mapper<boost::optional<T>>
max(const abstract_mapper<T> &m) {
    return function<boost::optional<T>>("max", m);
}

template<typename T>
exprn_mapper<boost::optional<T>>
min(const abstract_mapper<T> &m) {
    return function<boost::optional<T>>("min", m);
}

}

#endif
