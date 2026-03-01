//**************************************************************************/
// Copyright (c) 2018 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once
#include <assert1.h>
#include <vector>
#include <memory>

template <typename T>
class LinkedEntryTv2 : public MaxHeapOperators
{
public:
	T data;
	LinkedEntryTv2(T& d) {
		data = d;
	}
};

template <typename T, typename TE>
class LinkedListTv2 : public MaxHeapOperators
{
private:
	std::vector<std::shared_ptr<TE>> mData;
	static T mInvalidInstance;
public:
	LinkedListTv2() = default;

	~LinkedListTv2() {
		New();
	}

	void New()
	{
		for (auto i = 0; i < mData.size(); ++i) {
			mData[i].reset();
		}
		mData.clear();
	}

	int	Count() const {
		return static_cast<int>(mData.size());
	}

	void Append(T& item)
	{
		mData.push_back(std::make_shared<TE>(item));
	}

	T& operator[](int index)
	{
		const auto cnt = static_cast<int>(mData.size());
		DbgAssert((index >= 0 && index < cnt));
		if (index >= 0 && index < cnt) {
			return mData[index]->data;
		}
		// should never get here!
		return mInvalidInstance;
	}

};

template <typename T, typename TE>
T LinkedListTv2<T, TE>::mInvalidInstance;