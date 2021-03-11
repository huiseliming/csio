#pragma once

#define CSIO_FUNC (std::string(__FUNCTION__))
#define CSIO_LINE (std::to_string(__LINE__))
#define CSIO_FUNC_LINE (CSIO_FUNC + "(" + CSIO_LINE + ")")
