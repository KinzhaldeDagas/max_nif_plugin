//**************************************************************************/
// Copyright (c) 2013 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "Notifiers/MaxNotifier.h"

class Object;

namespace Max{ namespace NotificationAPI{
;

class MaxNodeNotifier : public MaxNotifier
{
public:
    explicit MaxNodeNotifier(INode& pNode, const NotifierType notifier_type);

    INode* GetMaxNode() const;
    NotifierType GetNotifierType() const;

    //From MaxNotifier
    virtual void DebugPrintToFile(FILE* f, size_t indent)const;

    //-- From ReferenceMaker
    virtual RefResult NotifyRefChanged(
        const Interval& changeInt, 
        RefTargetHandle hTarget, 
        PartID& partID, 
        RefMessage message,
        BOOL propagate );
    virtual int NumRefs();
    virtual RefTargetHandle GetReference(int i);

protected:

    // Protected destructor forces going through delete_this()
    virtual ~MaxNodeNotifier();

private:

    // Callback for RegisterNotification()
    static void ProcessNotifications(void* const param, NotifyInfo* const info); 

    void UpdateNodeCreateNotifier(bool on);

    INode* m_Node;
    INode* const m_PersistentNode;

    // Nodes map to multiple notifer types
    const NotifierType m_notifier_type;

    bool     m_TransformUpdateHasJustBeenCalled; //When the TM of a node is updated, you receive the PART_TM message then the PART_ALL, so we ignore the PART_ALL in that case
    int              GetUpdatedParamIDFromCamera ()const;//Returns LastNotifyParamNum (param block 1) from a Camera node
    static Object*   s_GetObject                 (INode *node, const SClass_ID& superClassID); //Returns the Object* from an INode* by taking care of XRef if any
    bool m_CreatingReference; // used in constructor when calling ReplaceReference to avoid taking this as a node restore
    bool m_NodeCreateNotifierRegistered = false;

    //from ReferenceMaker
    virtual void SetReference(int , RefTargetHandle rtarg);
};


} } // namespaces
