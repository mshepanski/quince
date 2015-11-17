//          Copyright Michael Shepanski 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file ../../LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <quince/detail/sql.h>
#include <quince/detail/util.h>
#include <quince/exprn_mappers/function.h>
#include <quince/query.h>

using std::string;
using std::unique_ptr;
using std::vector;


namespace quince {

unique_ptr<const abstract_expressionist>
make_function_call_expressionist(
    const string &function_name,
    vector<unique_ptr<const abstract_mapper_base>> &&args)
{
    struct expressionist : public abstract_expressionist {
        const string _function_name;
        const vector<const abstract_mapper_base *> _args;

        expressionist(const string &function_name, vector<unique_ptr<const abstract_mapper_base>> &&args) :
#ifdef __clang__
            _function_name(function_name.c_str()),
#else
            _function_name(function_name),
#endif
            _args(own_all(std::move(args)))
        {}

        virtual void
        write_expression(sql &cmd) const override {
            cmd.write_function_call(
                _function_name,
                transform(_args, [](const abstract_mapper_base *m)  { return &m->only_column();} )
            );
        }

        virtual column_id_set
        imports() const override {
            column_id_set result;
            for (const auto a: _args)  add_to_set(result, a->imports());
            return result;
        }
    };

    return quince::make_unique<expressionist>(function_name, std::move(args));
}

}
