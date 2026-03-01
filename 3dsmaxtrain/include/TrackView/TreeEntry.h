// ===========================================================================
// Copyright 2023 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================

//**************************************************************************/
// DESCRIPTION: TreeEntry class.
//		Part of core.dll.
// AUTHOR: Autodesk Inc.
//**************************************************************************/
#pragma once

#include "../Animatable.h"
#include "../maxtypes.h"

namespace MaxSDK
{
	/** Holds an Animatable handle, its clients handle and the subanim index such that: client->SubAnim(index) == anim.
	* Provides a series of useful methods to work with Animatables.
	* A valid TreeEntry can be just an animatable or just a client and subanim. Safely stores the animatables as handles to avoid dangling pointers.
	* Used by the trackviews tree view. */
	class CoreExport TreeEntry
	{
	public:
		/** Constructor for a TreeEntry.
		\param a The target Animatable.
		\param c The Animatables client (parent) in the scene. Important because most things only make sense in context with its parent.
		\param s Which sub Animatable 'a' is in 'c' such that: c->SubAnim(s) == a. */
		TreeEntry(Animatable* a, Animatable* c, int s)
			:mAnim(Animatable::GetHandleByAnim(a)),
			mClient(Animatable::GetHandleByAnim(c)),
			subNum(s)
		{}
		virtual ~TreeEntry() {}

		const Animatable* GetAnim() const { return Animatable::GetAnimByHandle(mAnim); }
		Animatable* GetAnim() { return Animatable::GetAnimByHandle(mAnim); }
		void SetAnim(Animatable* anim) { mAnim = Animatable::GetHandleByAnim(anim); }
		AnimHandle GetAnimHandle() const { return mAnim; }

		const Animatable* GetClient() const { return Animatable::GetAnimByHandle(mClient); }
		Animatable* GetClient() { return Animatable::GetAnimByHandle(mClient); }
		void SetClient(Animatable* client) { mClient = Animatable::GetHandleByAnim(client); }
		AnimHandle GetClientHandle() const { return mClient; }

		int GetSubNum() const { return subNum; }
		void SetSubNum(int n) { subNum = n; }

		/*! \brief Get the name of this entry to display in the UI.
		\param showType Whether or not to show the controller type in the name. */
		TSTR GetDisplayName(bool showType) const;

		//! \brief Determine the super class id, pulls type from paramblocks otherwise straight from the anim.
		SClass_ID GetEntrySuperClassID();
		//! \brief Returns true if this entry describes an easy curve controller.
		bool IsEaseCurve() const;
		//! \brief Returns true if this entry describes a multiplier curve controller.
		bool IsMultCurve() const;
		//! \brief Returns true if this entry describes a note track.
		bool IsNoteTrack() const;
		//! \brief Returns true if this entry describes a visibility controller.
		bool IsVisTrack() const;
		//! \brief Returns true if this entry describes a scene node.
		bool IsNode() const;
		//! \brief Returns true if this entry describes an object: geometry, camera, shape, helper, etc.
		bool IsObject() const;
		//! \brief Returns true if this entry describes a modifier.
		bool IsModifier() const;
		//! \brief Returns true if this entry or client describes a material or texmap.
		bool IsMtlBased() const;
		//! \brief Returns true if this entry has any sub animatables.
		virtual bool HasSubs() const;
		//! \brief Returns true if the entry represents a paramblock dummy.
		bool IsPBlockDummy() const;
		//! \brief Returns true if the entry represents a paramblock dummy. If true, 'sid' will be set to the dummy's parameter super class id.
		bool IsPBlockDummy(SClass_ID& sid) const;
		//! \brief Returns true if this entry is locked by the locked tracks manager (\ref ILockedTracksMan ).
		bool IsLocked();
		//! \brief Returns true if this entry is a source or destination of a parameter wire.
		bool IsParameterWired();
		//! \brief Returns true if this entry is able to be animated.
		bool IsPBlockDummyAnimatable() const;
		//! \brief Returns true if this entry describes a custom attribute.
		bool IsCustomAttribute() const;

		// required for using TreeEntry in std::unordered_set<>
		bool operator==(TreeEntry const& other) const
		{
			return mAnim == other.mAnim && mClient == other.mClient && subNum == other.subNum;
		}
	protected:
		AnimHandle mAnim;
		AnimHandle mClient;
		int subNum;
	};
} // namespace MaxSDK

namespace std {
template <>
struct hash<MaxSDK::TreeEntry>
{
	using argument_type = MaxSDK::TreeEntry;
	using result_type = size_t;

	result_type operator()(argument_type const& item) const
	{
		result_type hashValue = 127;
		hashValue = 73 * hashValue + std::hash<unsigned long long>{}(item.GetAnimHandle());
		hashValue = 73 * hashValue + std::hash<unsigned long long>{}(item.GetClientHandle());
		hashValue = 73 * hashValue + std::hash<int>{}(item.GetSubNum());
		return hashValue;
	}
};
} // namespace std