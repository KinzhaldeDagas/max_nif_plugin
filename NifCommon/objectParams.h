/*===========================================================================*\
 |
 |  FILE:    objectParams.h
 |           Skeleton project and code for a Utility
 |           3D Studio MAX R3.0
 |
 |  AUTH:    Cleve Ard
 |           Render Group
 |           Copyright(c) Discreet 2000
 |
\*===========================================================================*/

#ifndef _OBJECTPARAMS_H_
#define _OBJECTPARAMS_H_

#include "assert1.h"

#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/name.h>
#include <maxscript/foundation/colors.h>
#include <maxscript/maxwrapper/mxsobjects.h>
#include <maxscript/macros/value_locals.h>

template <class T>
class ConvertMAXScriptToC;

inline Value* make_maxscript_value(bool v);
inline Value* make_maxscript_value(int v);
inline Value* make_maxscript_value(float v);
inline Value* make_maxscript_value(COLORREF rgb);
inline Value* make_maxscript_value(const Color& rgb);
inline Value* make_maxscript_value(LPCTSTR str);
inline Value* make_maxscript_value(ReferenceTarget* rtarg);

namespace ObjectParamsDetail
{
	inline void ResetMaxScriptStateOnError() noexcept
	{
		clear_error_source_data();
		MAXScript_signals = 0;
		if (progress_bar_up)
		{
			MAXScript_interface->ProgressEnd();
			progress_bar_up = FALSE;
		}
	}

	struct MxsContext final
	{
		ScopedMaxScriptEvaluationContext eval;
		ScopedSaveCurrentFrames savedFrames;
		ScopedErrorTracebackDisable disableTraceback;

		explicit MxsContext(BOOL disableTrace = TRUE)
			: eval(true, nullptr)
			, savedFrames(eval.Get_TLS())
			, disableTraceback(disableTrace, eval.Get_TLS())
		{
		}

		MAXScript_TLS* tls() noexcept { return eval.Get_TLS(); }

		MxsContext(const MxsContext&) = delete;
		MxsContext& operator=(const MxsContext&) = delete;
	};
}

template<class T>
bool setMAXScriptValue(ReferenceTarget* obj, LPTSTR name, TimeValue t, T value, int tabIndex)
{
	bool rval = false;
	assert(obj != NULL);

	ObjectParamsDetail::MxsContext ctx(TRUE);
	ScopedValueTempArray vl(2, ctx.tls());
	Value** locals = vl.data();

	try
	{
		locals[0] = Name::intern(name);
		locals[1] = MAXWrapper::get_property(obj, locals[0], NULL);

		if (locals[1] != NULL && locals[1] != &undefined && locals[1] != &unsupplied && is_tab_param(locals[1]))
		{
			MAXPB2ArrayParam* array = static_cast<MAXPB2ArrayParam*>(locals[1]);
			if (array->pblock != NULL && array->pdef != NULL)
			{
				const int count = array->pblock->Count(array->pdef->ID);
				if (tabIndex >= 0 && tabIndex < count)
				{
					array->pblock->SetValue(array->pdef->ID, t, value, tabIndex);
					rval = true;
				}
			}
		}
	}
	catch (...)
	{
		ObjectParamsDetail::ResetMaxScriptStateOnError();
	}

	return rval;
}

template<class T>
bool setMAXScriptValue(ReferenceTarget* obj, LPTSTR name, TimeValue, T& value)
{
	bool rval = false;
	assert(obj != NULL);

	ObjectParamsDetail::MxsContext ctx(TRUE);
	ScopedValueTempArray vl(2, ctx.tls());
	Value** locals = vl.data();

	try
	{
		locals[0] = Name::intern(name);
		locals[1] = make_maxscript_value(value);

		Value* result = MAXWrapper::set_property(obj, locals[0], locals[1]);
		rval = (result != NULL);
	}
	catch (...)
	{
		ObjectParamsDetail::ResetMaxScriptStateOnError();
	}

	return rval;
}

template<class T>
bool getMAXScriptValue(ReferenceTarget* obj, LPTSTR name, TimeValue t, T& value, int tabIndex)
{
	bool rval = false;
	assert(obj != NULL);

	ObjectParamsDetail::MxsContext ctx(TRUE);
	ScopedValueTempArray vl(2, ctx.tls());
	Value** locals = vl.data();

	try
	{
		locals[0] = Name::intern(name);
		locals[1] = MAXWrapper::get_property(obj, locals[0], NULL);

		if (locals[1] != NULL && locals[1] != &undefined && locals[1] != &unsupplied && is_tab_param(locals[1]))
		{
			MAXPB2ArrayParam* array = static_cast<MAXPB2ArrayParam*>(locals[1]);
			if (array->pblock != NULL && array->pdef != NULL)
			{
				const int count = array->pblock->Count(array->pdef->ID);
				if (tabIndex >= 0 && tabIndex < count)
				{
					array->pblock->GetValue(array->pdef->ID, t, value, Interval(0, 0), tabIndex);
					rval = true;
				}
			}
		}
	}
	catch (...)
	{
		ObjectParamsDetail::ResetMaxScriptStateOnError();
	}

	return rval;
}

template<class T>
bool getMAXScriptValue(ReferenceTarget* obj, LPTSTR name, TimeValue, T& value)
{
	bool rval = false;
	assert(obj != NULL);

	ObjectParamsDetail::MxsContext ctx(TRUE);
	ScopedValueTempArray vl(2, ctx.tls());
	Value** locals = vl.data();

	try
	{
		locals[0] = Name::intern(name);
		locals[1] = MAXWrapper::get_property(obj, locals[0], NULL);

		if (locals[1] != NULL && locals[1] != &undefined && locals[1] != &unsupplied)
		{
			value = ConvertMAXScriptToC<T>::cvt(locals[1]);
			rval = true;
		}
	}
	catch (...)
	{
		ObjectParamsDetail::ResetMaxScriptStateOnError();
	}

	return rval;
}

inline Control* getMAXScriptController(ReferenceTarget* obj, LPTSTR name, ParamDimension*& dim)
{
	Control* rval = NULL;
	assert(obj != NULL);

	ObjectParamsDetail::MxsContext ctx(TRUE);
	ScopedValueTempArray vl(1, ctx.tls());
	Value** locals = vl.data();

	try
	{
		locals[0] = Name::intern(name);
		rval = MAXWrapper::get_max_prop_controller(obj, locals[0], &dim);
	}
	catch (...)
	{
		ObjectParamsDetail::ResetMaxScriptStateOnError();
	}

	return rval;
}

inline bool setMAXScriptController(ReferenceTarget* obj, LPTSTR name, Control* control, ParamDimension* dim)
{
	bool rval = false;
	assert(obj != NULL);

	ObjectParamsDetail::MxsContext ctx(TRUE);
	ScopedValueTempArray vl(2, ctx.tls());
	Value** locals = vl.data();

	try
	{
		locals[0] = Name::intern(name);
		locals[1] = MAXControl::intern(control, dim);
		rval = (MAXWrapper::set_max_prop_controller(obj, locals[0], static_cast<MAXControl*>(locals[1])) != 0);
	}
	catch (...)
	{
		ObjectParamsDetail::ResetMaxScriptStateOnError();
	}

	return rval;
}

inline Value* make_maxscript_value(bool v)
{
	return v ? &true_value : &false_value;
}

inline Value* make_maxscript_value(int v)
{
	return Integer::intern(v);
}

inline Value* make_maxscript_value(float v)
{
	return Float::intern(v);
}

inline Value* make_maxscript_value(COLORREF rgb)
{
	return new ColorValue(rgb);
}

inline Value* make_maxscript_value(const Color& rgb)
{
	return new ColorValue(rgb);
}

inline Value* make_maxscript_value(LPCTSTR str)
{
	return Name::intern(const_cast<LPTSTR>(str));
}

inline Value* make_maxscript_value(ReferenceTarget* rtarg)
{
	return MAXClass::make_wrapper_for(rtarg);
}

template <class T>
class ConvertMAXScriptToC
{
public:
	static T cvt(Value* val);
};

template <>
inline bool ConvertMAXScriptToC<bool>::cvt(Value* val)
{
	return val->to_bool() != 0;
}

template <>
inline int ConvertMAXScriptToC<int>::cvt(Value* val)
{
	return val->to_int();
}

template <>
inline float ConvertMAXScriptToC<float>::cvt(Value* val)
{
	return val->to_float();
}

template <>
inline Color ConvertMAXScriptToC<Color>::cvt(Value* val)
{
	return val->to_point3();
}

template <>
inline LPCTSTR ConvertMAXScriptToC<LPCTSTR>::cvt(Value* val)
{
	return val->to_string();
}

template <>
inline TSTR ConvertMAXScriptToC<TSTR>::cvt(Value* val)
{
	return val->to_string();
}

template <>
inline Texmap* ConvertMAXScriptToC<Texmap*>::cvt(Value* val)
{
	return val->to_texmap();
}

template <>
inline ReferenceTarget* ConvertMAXScriptToC<ReferenceTarget*>::cvt(Value* val)
{
	return val->to_reftarg();
}

#endif
