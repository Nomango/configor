#pragma once

#include "configor.hpp"

namespace configor
{

template <class TplArgs>
class basic_json final
{
public:
    using value = basic_configor<TplArgs>;
};

using json  = basic_json<configor_tpl_args>;
using wjson = basic_json<wconfigor_tpl_args>;

}  // namespace configor
