/*****************************************************************************

Project:  Juice
Module:   JuicePlay.exe
Filename: ModelTreeView.h

Copyright (c) 2001-2003 Nate Waddoups, coding@natew.com, except as noted

JuicePlay.exe and its source code are distributed under the terms of the 
Juice license.  The terms of the Juice license are available at
http://www.natew.com/juice/License.html

*****************************************************************************/

#pragma once

using namespace Juice;

// CModelTreeView view

class CModelTreeView : public CView,
	public Generic::Listener <ModelEvents>,
	public Generic::Listener <ComponentEvents>,
	public Generic::Listener <BehaviorEvents>,
	public Generic::Listener <ClockworkBehaviorEvents>,
	public Generic::Listener <ClockworkMotorEvents>,
	public Generic::Listener <SelectionSetEvents>	
{
	DECLARE_DYNCREATE(CModelTreeView)

	CModelTreeView();           // protected constructor used by dynamic creation
	virtual ~CModelTreeView();

	const static int iTreeCtrlID = 101;
	const static int UWM_TV_CHECKBOX = WM_APP;

protected:

	Model *m_pModel;
	CTreeCtrl m_Tree;
	HTREEITEM m_htiBodies, m_htiRevoluteJoints, m_htiPrismaticJoints, m_htiSphericalJoints, m_htiBehaviors;

	void vRepaintTree ();

	bool fGetCheck (HTREEITEM hItem);
	void vSetCheck (HTREEITEM hItem, bool fCheck);

	void vHideCheckBox (HTREEITEM hItem);

	// recursive tree item manipulation: return true if recursion should continue, false if not
	typedef void (CModelTreeView::*fnTreeItemAction) (HTREEITEM hItem, void *pItemContents, void *pData);
	void vFindMatchingItems (void *pItemContents, fnTreeItemAction fn, void *pFnData = null);
	void vRecursiveFindMatchingItems (HTREEITEM hItem, void *pItemContents, fnTreeItemAction fn, void *pFnData);

	// selection set management
	virtual void vResetSelectionSet ();

	// overrideables for the behavior tree view class
	virtual void vSelectMotor (ClockworkMotor *pMotion);
	virtual void vSelectBehavior (Behavior *pBehavior);

	// tree search actions
	void vDeleteEverything  (HTREEITEM hItem, void *pItemContents, void *pData);
	void vDelete            (HTREEITEM hItem, void *pItemContents, void *pData);
	void vComponentChange   (HTREEITEM hItem, void *pItemContents, void *pData);
	void vBehaviorChange    (HTREEITEM hItem, void *pItemContents, void *pData);
	void vMotorChange  (HTREEITEM hItem, void *pItemContents, void *pData);

	void vSelect            (HTREEITEM hItem, void *pItemContents, void *pData); 
	void vComponentSelect   (HTREEITEM hItem, void *pItemContents, void *pData);
	void vBehaviorSelect    (HTREEITEM hItem, void *pItemContents, void *pData);
	void vMotorSelect  (HTREEITEM hItem, void *pItemContents, void *pData);
	void vGetItem           (HTREEITEM hItem, void *pItemContents, void *pDesiredItem);

	// notification event handlers
	void Model_AddComponent     (Model *pModel, Component *pComponent);
	void Model_DeleteComponent  (Model *pModel, Component *pComponent);
	void Model_AddBehavior      (Model *pModel, Behavior *pBehavior);
	void Model_DeleteBehavior   (Model *pModel, Behavior *pBehavior);
    
	void Component_Changed       (Component *pComponent);
    void Notifier_Deleted       (Component *pComponent);

	void Behavior_Changed        (Behavior *pBehavior);
	void ClockworkBehavior_AddMotor (ClockworkBehavior *pBehavior, ClockworkMotor *pMotor);
	void Notifier_Deleted       (Behavior *pBehavior);	

	void ClockworkMotor_Changed (ClockworkMotor *pMotor);
	void Notifier_Deleted       (ClockworkMotor *pMotor);

	virtual void Selection_Add    (SelectionSet *pSet, Selectable *pSelectable, bool fAutomatic);
	virtual void Selection_Remove (SelectionSet *pSet, Selectable *pSelectable);

public:
	void vSetModel (Model *pModel);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

	virtual void OnDraw(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnDestroy();

	afx_msg LRESULT OnPostCheckBoxOperation (WPARAM wp, LPARAM lp);
};

class CBehaviorTreeView : public CModelTreeView
{
protected:
	DECLARE_DYNCREATE(CBehaviorTreeView)
	DECLARE_MESSAGE_MAP()

	// selection set management
	virtual void vResetSelectionSet ();

	// base class overrides
	virtual void vSelectMotor (ClockworkMotor *pMotion);
	virtual void vSelectBehavior (Behavior *pBehavior);

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void Model_AddComponent     (Model *pModel, Component *pComponent) {}

	// I'm not sure why, but this doesn't get called - the base class version gets called instead
	virtual void Selection_Remove (SelectionSet *pSet, Selectable *pSelectable);

	void Notifier_Deleted       (Behavior *pBehavior);
	void Notifier_Deleted       (ClockworkMotor *pMotor);
};

/****************************************************************************/
/** 
*****************************************************************************/

class TreeItemData
{
	Selectable *m_pSelectable;
	Component *m_pComponent;
	Behavior *m_pBehavior;
	ClockworkMotor *m_pMotor;
public:

	enum Type
	{
		tInvalid,
		tComponent,
		tBehavior,
		tMotor
	};

	TreeItemData (Component *pComponent) :
		m_eType (tComponent),
		m_pSelectable (pComponent),
		m_pComponent (pComponent),
		m_pBehavior (null),
		m_pMotor (null) {}

	TreeItemData (Behavior *pBehavior) :
		m_eType (tBehavior),
		m_pSelectable (pBehavior),
		m_pComponent (null),
		m_pBehavior (pBehavior),
		m_pMotor (null) {}

	TreeItemData (ClockworkMotor *pMotor) :
		m_eType (tMotor),
		m_pSelectable (pMotor),
		m_pComponent (null),
		m_pBehavior (null),
		m_pMotor (pMotor) {}

	Selectable *pSelectable () { return m_pSelectable; }
	Component *pComponent () { return m_pComponent; }
	Behavior *pBehavior () { return m_pBehavior; }
	ClockworkMotor *pMotor () { return m_pMotor; }

	void* pWhatever () 
	{
		switch (m_eType)
		{
		case tComponent:
			return m_pComponent;

		case tBehavior:
			return m_pBehavior;

		case tMotor:
			return m_pMotor;
		}

		return null;
	}

	Type eGetType () { return m_eType; }

protected:
	Type m_eType;
};

// ModelTreeView.h ends here ->