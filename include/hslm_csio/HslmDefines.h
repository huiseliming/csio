#pragma once

#define HSLM_FUNC (std::string(__FUNCTION__))
#define HSLM_LINE (std::to_string(__LINE__))
#define HSLM_FUNC_LINE (HSLM_FUNC + "(" + HSLM_LINE + ")")
