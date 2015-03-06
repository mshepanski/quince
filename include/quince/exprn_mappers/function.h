#ifndef QUINCE__exprn_mappers__function_h
#define QUINCE__exprn_mappers__function_h

//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/exprn_mappers/detail/exprn_mapper.h>


namespace quince {

std::unique_ptr<const abstract_expressionist>
make_function_call_expressionist(
    const std::string &function_name,
    std::vector<std::unique_ptr<const abstract_mapper_base>> &&args
);

template<typename T>
exprn_mapper<T>
function(
    const std::string &function_name,
    const std::vector<const abstract_mapper_base *> &args
) {
    return exprn_mapper<T>(make_function_call_expressionist(
        function_name,
        clone_all(args)
    ));
}

template<typename T, typename Arg, typename... Args>
exprn_mapper<T>
function(
    const std::string &function_name,
    const Arg &arg,
    const Args &... args
) {
    return exprn_mapper<T>(make_function_call_expressionist(
        function_name,
        make_new_mappers(arg, args...)
    ));
}

}

#endif
