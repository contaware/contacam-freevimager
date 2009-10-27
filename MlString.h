#pragma once

#define ML_STRING(id,text) ml_string(id)
#define ML_RESOURCE(id) ml_string(id)
extern const CString ml_string(int StringId);