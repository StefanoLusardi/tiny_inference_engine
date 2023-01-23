#include <tie_client/data_type.hpp>

namespace tie::client
{
[[nodiscard]] 
data_type::value data_type::from_string(const std::string& str_value)
{
    if (auto str_type = _string_to_type.find(str_value); str_type != _string_to_type.end())
        return str_type->second;

    return data_type::Unknown;
}

[[nodiscard]]
std::string data_type::str() const
{
    return _type_to_string.at(_value);
}

std::map<data_type::value, std::string> data_type::_type_to_string
{
    { data_type::Bool,    "BOOL"    },
    { data_type::Uint8,   "UINT8"   },
    { data_type::Uint16,  "UINT16"  },
    { data_type::Uint32,  "UINT32"  },
    { data_type::Uint64,  "UINT64"  },
    { data_type::Int8,    "INT8"    },
    { data_type::Int16,   "INT16"   },
    { data_type::Int32,   "INT32"   },
    { data_type::Int64,   "INT64"   },
    { data_type::Fp16,    "FP16"    },
    { data_type::Fp32,    "FP32"    },
    { data_type::Fp64,    "FP64"    },
    { data_type::String,  "STRING"  },
    { data_type::Unknown, "Unknown" }
};

std::map<std::string, data_type::value> data_type::_string_to_type
{
    { "BOOL"    ,  data_type::Bool    },
    { "UINT8"   ,  data_type::Uint8   },
    { "UINT16"  ,  data_type::Uint16  },
    { "UINT32"  ,  data_type::Uint32  },
    { "UINT64"  ,  data_type::Uint64  },
    { "INT8"    ,  data_type::Int8    },
    { "INT16"   ,  data_type::Int16   },
    { "INT32"   ,  data_type::Int32   },
    { "INT64"   ,  data_type::Int64   },
    { "FP16"    ,  data_type::Fp16    },
    { "FP32"    ,  data_type::Fp32    },
    { "FP64"    ,  data_type::Fp64    },
    { "STRING"  ,  data_type::String  },
    { "Unknown" ,  data_type::Unknown }
};

}