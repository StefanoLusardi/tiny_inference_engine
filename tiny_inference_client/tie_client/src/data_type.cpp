#include <tie_client/data_type.hpp>

namespace tie::client
{
[[nodiscard]] 
data_type::Value data_type::from_string(const char* str_value)
{
    if (string_to_type.find(str_value) != string_to_type.end())
        return string_to_type.at(str_value);

    return data_type::Unknown;
}

[[nodiscard]]
const char* data_type::str() const
{
    return type_to_string.at(value);
}

std::map<data_type::Value, const char*> data_type::type_to_string
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

std::map<const char*, data_type::Value> data_type::string_to_type
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