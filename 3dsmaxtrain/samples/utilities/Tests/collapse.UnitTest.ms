/*
	@param TestName:	Collapse
	@param Author:	 Jesse Desmarais
	@param Created:  05/16/2022
	@param LastMod:	 05/16/2022
	@param ModBy:	 Jesse Desmarais
	@param Desc:	 Collapse utilities maxscrpt Unit tests
	@param Notes:   
	@param Status:	  Online
*/

fileIn "MxsUnit/MxsUnit.ms"

struct util_collapse_test (

    private 
    function mergeInForCompare requestedNodeName = (
        
        filePath = getFileNamePath (getSourceFileName())
		fileName = filePath + "\\TestData\\Bool_compare.max"
        res = mergemaxfile fileName #(requestedNodeName)
        logMsg = requestedNodeName + " was not found in " + fileName
        assert_true res message: logMsg
    ),

    function CollapseCompareTwoNodes baseNode testNode  testName = (
        logMsg = "numverts from baseNode and testNode do not match in " + testName
        assert_equal baseNode.numverts testNode.numverts message:logMsg

        for i = 1 to baseNode.numverts do (
            baseVert = getVert baseNode i
            testVert = getVert testNode i

            assert_point3_equal baseVert testVert
        )
    ),

    function CheckCollapseType node expectedClass testName = (
        classOfCollapse = classof node == expectedClass
        logMsg = testName + " test did not collapse to correct class type expected" + (expectedClass as String)
        
        assert_true classOfCollapse message:logMsg
    ),


    public
    	function setup =
	(
	),

	function teardown =
	(

	),


    function modifierstack_collapse_test = (
        
        collapse.setoutputtype #stackresult
        outputType = collapse.getoutputtype()
        assert_equal #stackresult outputType  message:"outputType is incorrect expected #stackResult"
        
        mergeInForCompare "SingleSource"

        collapse.docollapse($SingleSource)
        assert_equal 0 $SingleSource.modifiers.count message:"There are still modifiers post collapse"
        
        CheckCollapseType $SingleSource Editable_Poly "modifier stack "
        mergeInForCompare "SingleStack"


        assert_equal $SingleStack.numverts $SingleSource.numverts message:"numverts from baseNode and testNode do not match in modifierStack collapse test"
        for i = 1 to $SingleStack.numverts do (
            baseVert = polyop.getVert $SingleStack i
            testVert = polyop.getVert $SingleSource i

            assert_point3_equal baseVert testVert
        )
    ),

    function mesh_collapse_single = (
        resetMaxFile #noPrompt
        collapse.setoutputtype #mesh
        outputType = collapse.getoutputtype()
        assert_equal #mesh outputType  message:"outputType is incorrect expected #mesh"    
        
        collapse.setcollapseto #single
        collapseToType = collapse.getcollapseto()
        assert_equal #single collapseToType message:"collapseTo Type is incorrect expected #single"

        mergeInForCompare "SingleSource"

        collapse.docollapse($SingleSource)
        mergeInForCompare "SingleMesh001"
        CheckCollapseType $SingleSource Editable_mesh "single Mesh from single source"
        CollapseCompareTwoNodes $SingleMesh001 $SingleSource "single mesh from single source"

        mergeInForCompare "MultipleSource001"
        mergeInForCompare "MultipleSource002"
        mergeInForCompare "MultipleSource003"

        collapse.docollapse(#($multipleSource001,$multipleSource002,$multipleSource003))
        CheckCollapseType $multipleSource003 Editable_mesh "single Mesh from mulitple source"
        mergeInForCompare "SingleMesh002"
        CollapseCompareTwoNodes $SingleMesh002 $multipleSource003 "single mesh from multiple source"

    ),

    function mesh_collapse_multiple = (
        resetMaxFile #noprompt
        collapse.setoutputtype #mesh
        outputType = collapse.getoutputtype()
        assert_equal #mesh outputType  message:"outputType is incorrect expected #mesh"    
        
        collapse.setcollapseto #multiple
        collapseToType = collapse.getcollapseto()
        assert_equal #multiple collapseToType message:"collapseTo Type is incorrect expected #multiple"

        mergeInForCompare "MultipleSource001"
        mergeInForCompare "MultipleSource002"
        mergeInForCompare "MultipleSource003"

        collapse.docollapse($geometry)
        assert_equal 3 $geometry.count message:"There is less objects after mulitple collapse"
        CheckCollapseType $multipleSource001 Editable_mesh "mesh 1 from mulitple mesh from multiple collapse"
        CheckCollapseType $multipleSource002 Editable_mesh "mesh 1 from mulitple mesh from multiple collapse"
        CheckCollapseType $multipleSource003 Editable_mesh "mesh 1 from mulitple mesh from multiple collapse"

        mergeInForCompare "MultipleMesh001" 
        mergeInForCompare "MultipleMesh002"
        mergeInForCompare "MultipleMesh003"

        CollapseCompareTwoNodes $MultipleMesh001 $multipleSource001 "mesh001 for multiple mesh from multiple source"
        CollapseCompareTwoNodes $MultipleMesh002 $multipleSource002 "mesh002 for multiple mesh from multiple source"
        CollapseCompareTwoNodes $MultipleMesh003 $multipleSource003 "mesh003 for multiple mesh from multiple source"




    ),

    function mesh_collapse_bool = (
        resetMaxFile #noprompt

        --Union
        testPlane = plane lengthSegs:14 widthSegs:14 length:200 width:100
        testCylinder = cylinder position:[0,0,-50] height:100 radius:50
        convertToPoly testPlane
        convertToPoly testCylinder


        expectedNoBooleanVertCount =  testPlane.numverts + testCylinder.numverts
        collapse.setdobool false
        collapse.setoutputtype #mesh
        collapse.setcollapseto #single

        doBool = collapse.getdobool()
        outputType = collapse.getoutputtype()
        collapseToType = collapse.getcollapseto()

        assert_equal false doBool message:"getdoBool not correct in pre bool union test expected false"
        assert_equal #mesh outputType message: "output type not correct in pre bool union test expected #mesh"
        assert_equal #single collapseToType message: "collapse to not correct in pre bool union test expected #single"

        collapse.docollapse #(testPlane,testCylinder)
        assert_equal testPlane.numverts expectedNoBooleanVertCount message:"collapse produced incorrect number of verts in pre union test"
        resetMaxFile #noprompt

        collapse.setdobool true;
        collapse.setboolType #Union
        doBool = collapse.getdoBool()
        boolOp = collapse.getBoolType()
        assert_equal true doBool message:"getdoBool not correct in bool union test expected true"
        assert_equal #union boolOp  message:"getBoolType not correct in bool union test expected #union"
        
        testPlane = plane lengthSegs:14 widthSegs:14 length:200 width:100
        testCylinder = cylinder position:[0,0,-50] height:100 radius:50
        convertToPoly testPlane
        convertToPoly testCylinder
        
        collapse.docollapse #(testPlane,testCylinder)
        
        mergeInForCompare "Union"
        CollapseCompareTwoNodes $Union testPlane "Union"
        

        --Intersection 
        resetMaxFile #noprompt
        collapse.setdobool true;
        collapse.setboolType #intersection
        doBool = collapse.getdoBool()
        boolOp = collapse.getBoolType()
        assert_equal true doBool message:"getdoBool not correct in bool intersection test expected true"
        assert_equal #intersection boolOp  message:"getBoolType not correct in bool union test expected #intersection"
        testPlane = plane lengthSegs:14 widthSegs:14 length:200 width:100
        testCylinder = cylinder position:[0,0,-50] height:100 radius:50
        convertToPoly testPlane
        convertToPoly testCylinder
        collapse.docollapse #(testPlane,testCylinder)
        
        mergeInForCompare "Intersection"
        CollapseCompareTwoNodes $Intersection testPlane "Intersection"

        --subtraction

        resetMaxFile #noprompt

        collapse.setdobool true;
        collapse.setboolType #subtraction
        doBool = collapse.getdoBool()
        boolOp = collapse.getBoolType()
        assert_equal true doBool message:"getdoBool not correct in bool subtraction test expected true"
        assert_equal #subtraction boolOp  message:"getBoolType not correct in bool union test expected #subtraction"       
        
        testSphere = sphere segments:30
        subtractSphere = sphere segments:30 pos:[-20,0,0]
        collapse.docollapse #(testSphere, subtractSphere)
        
        mergeInForCompare "Subtraction"
        CollapseCompareTwoNodes $Subtraction testSphere "Subtraction"
    )
    ,

    Tests = 
    #(
        modifierstack_collapse_test,
        mesh_collapse_single,
        mesh_collapse_multiple,
        mesh_collapse_bool
	)
)

run_test_fixture util_collapse_test script:(getThisScriptFilename())
