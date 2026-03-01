define_struct_global ( _T("numViews"),				_T("viewport"),				VP_GetNumViews,				VP_SetNumViews);
define_struct_global ( _T("activeViewport"),		_T("viewport"),				VP_GetActiveViewport,		VP_SetActiveViewport);
define_struct_global (_T("activeViewportId"),		_T("viewport"),				VP_GetActiveViewportId,		VP_SetActiveViewportId);

// define_struct_global ( "gridSize",				"viewport",				VP_GetGridSize,				VP_SetGridSize);
define_struct_global ( _T("DispBkgImage"),			_T("viewport"),				VP_GetBkgImageDsp,			VP_SetBkgImageDsp);

// Viewport picking selection radius accessors
define_struct_global ( _T("pickingSelectionRadiusScale"),	_T("viewport"),		VP_GetPickingSelectionRadiusScale,		VP_SetPickingSelectionRadiusScale);
define_struct_global ( _T("pickingSelectionRadius"),		_T("viewport"),		VP_GetPickingSelectionRadius,			VP_SetPickingSelectionRadius);
