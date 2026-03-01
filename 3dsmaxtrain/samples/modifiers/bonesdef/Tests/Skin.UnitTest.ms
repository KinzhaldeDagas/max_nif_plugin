/*
    @param TestName:    Modifiers Skin Unit Test
    @param Author:      Peter Watje
    @param Created:     March 2013
    @param LastMod:     2024-04-21
    @param ModBy:       Larry Minton
    @param Description: Misc Modifiers Skin Unit Tests
    @param Status:      Online
    @param Tags:
*/

fileIn "MxsUnit/MxsUnit.ms"

struct Obj_Prop_UnitTest
(
private


	--##################################################
	-- Helper functions
	--##################################################


	function validateProperties experimental expected =
	(
		if (experimental.count != expected.count) then
		(
			assert_equal expected.count experimental.count message:"Incorrect number of properties"
		)
		else
		(
			for i in 1 to experimental.count do
			(
				assert_equal expected[i] experimental[i] message:("Incorrect Value for " + testprops[i])
			)
		)
	),

	function changeValue valueName pvalue properties =
	(
		for i in 1 to properties.count do
		(
			if testprops[i] == valueName do
			(
				properties[i] = pvalue
				return true
			)
		)
		local str = stringstream ""
		format "Value(%): %, not found in property array" valueName pvalue to:str
		assert_true false message:(str as string)
	),

	--##################################################
	-- Unit tests
	--##################################################
public
	function setup =
	(
		resetMaxFile #noPrompt
	),

	function teardown =
	(
		
	),
/*
	Regression test for  MAXX_11350
	Max script index issue
	Fix : the code as thinking the index was 1 based, set it to zero base
*/
	function skin_MAXX_11350 =
	(
		resetMaxFile #noPrompt
		Cylinder smooth:on heightsegs:5 capsegs:1 sides:18 height:57.0311 radius:11.2255 mapcoords:on pos:[-1.18015,0.751877,0] isSelected:on
		Dummy transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [-0.879765,0,1.46628]) isSelected:on
		Dummy transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [-0.879765,-1.2434e-006,28.4457]) isSelected:on
		Dummy transform:(matrix3 [1,0,0] [0,0,1] [0,-1,0] [-0.293255,-2.3458e-006,53.6657]) isSelected:on

		local testNode = $Cylinder001
		local testMod = Skin()
		addModifier testNode testMod
		clearSelection()

		skinOps.addBone testMod $Dummy001 1 node:testNode
		skinOps.addBone testMod $Dummy002 1 node:testNode
		skinOps.addBone testMod $Dummy003 1 node:testNode
		
		forceCompleteRedraw()

		iret = false
			
		skinOps.rigidVertex testMod 1 false node:testNode
		iret = skinOps.isRigidVertex testMod 1 node:testNode
		assert_true (iret == 0)
			
		skinOps.rigidVertex testMod 1 true node:testNode
		iret = skinOps.isRigidVertex testMod 1 node:testNode
		assert_true (iret == 1)

		skinOps.rigidVertex testMod 2 false node:testNode
		iret = skinOps.isRigidVertex testMod 2 node:testNode
		assert_true (iret == 0)

		skinOps.rigidVertex testMod 2 true node:testNode
		iret = skinOps.isRigidVertex testMod 2 node:testNode
		assert_true (iret == 1)
	
		
		skinOps.rigidHandle testMod 1 false node:testNode
		iret = skinOps.isRigidHandle testMod 1 node:testNode
		assert_true (iret == 0)

		skinOps.rigidHandle testMod 1 true node:testNode
		iret = skinOps.isRigidHandle testMod 1 node:testNode
		assert_true (iret == 1)

		skinOps.rigidHandle testMod 2 false node:testNode
		iret = skinOps.isRigidHandle testMod 2 node:testNode
		assert_true (iret == 0)

		skinOps.rigidHandle testMod 2 true node:testNode
		iret = skinOps.isRigidHandle testMod 2 node:testNode
		assert_true (iret == 1)
			
			
		skinOps.UnNormalizeVertex testMod 1 false node:testNode
		iret = skinOps.isUnNormalizeVertex testMod 1 node:testNode

		assert_true (iret == 0)

		skinOps.UnNormalizeVertex testMod 1 true node:testNode
		iret = skinOps.isUnNormalizeVertex testMod 1 node:testNode
		assert_true (iret == 1)

		skinOps.UnNormalizeVertex testMod 2 false node:testNode
		iret = skinOps.isUnNormalizeVertex testMod 2 node:testNode
		assert_true (iret == 0)

		skinOps.UnNormalizeVertex testMod 2 true node:testNode
		iret = skinOps.isUnNormalizeVertex testMod 2 node:testNode
		assert_true (iret == 1)
		
	),

	function skin_DQ_UnitTest =
	(
		resetMaxFile #noPrompt
		Cylinder smooth:on heightsegs:5 capsegs:1 sides:8 height:56.8222 radius:10.9067 mapcoords:on pos:[-0.902115,1.30113,0] isSelected:on
		Box lengthsegs:1 widthsegs:1 heightsegs:1 length:15.2588 width:15.41124 height:30.8898 mapcoords:on pos:[1.39257,0.63385,0] isSelected:on
		Box lengthsegs:1 widthsegs:1 heightsegs:1 length:15.34133 width:17.71352 height:31.4761 mapcoords:on pos:[1.39257,0.63385,31] isSelected:on
		Select $BOx002
		$.parent = $Box001
		sliderTime = 20f
		set animate on
		rotate $ (angleaxis -95.0535 [0,1,0])
		set animate off
		sliderTime = 0f
		local testNode = $Cylinder001
		local testMod = Skin()
		addModifier testNode testMod
		clearSelection()

		skinOps.addBone testMod $Box001 1 node:testNode
		skinOps.addBone testMod $Box002 1 node:testNode
		testMod.cross_radius = 28.1854
		sliderTime = 20f
		numVerts = meshop.getNumVerts testNode
		for i = 1 to numVerts/2 do
		(
			skinops.setVertexDQWeight testMod i 0.5 node:testNode
		)

		for i = numVerts/2 to numVerts do
		(
			skinops.setVertexDQWeight testMod i 1.0 node:testNode
		)
		testMod.enableDQ = off



		testMod.enableDQ = on



	),
	
	function skin_VoxelHeatMap_UnitTest =
	(

		if systemTools.IsGpuPresent() then 
		(
            resetMaxFile #noPrompt
            max create mode
            Cylinder smooth:on heightsegs:5 capsegs:1 sides:8 height:56.8222 radius:10.9067 mapcoords:on pos:[-0.902115,1.30113,0] isSelected:on
            Box lengthsegs:1 widthsegs:1 heightsegs:1 length:15.2588 width:15.41124 height:30.8898 mapcoords:on pos:[1.39257,0.63385,0] isSelected:on
            Box lengthsegs:1 widthsegs:1 heightsegs:1 length:15.34133 width:17.71352 height:31.4761 mapcoords:on pos:[1.39257,0.63385,31] isSelected:on
            Select $BOx002
            $.parent = $Box001
            sliderTime = 20f
            set animate on
            rotate $ (angleaxis -95.0535 [0,1,0])
            set animate off
            sliderTime = 0f
            local testNode = $Cylinder001
            local testMod = Skin()
            addModifier testNode testMod
            clearSelection()
            
            skinOps.addBone testMod $Box001 1 node:testNode
            skinOps.addBone testMod $Box002 1 node:testNode
            max create mode
            max modify mode
            	
            format "heatmap falloff			\n"
            skinOps.voxelWeighting testMod 1 0.0 -1 0 off on node:testNode
            max create mode
            max modify mode
            
            
            skinOps.voxelWeighting testMod 1 1.0 -1 0 off on node:testNode
            max create mode
            max modify mode
            
            format "heatmap max influence		\n"
            skinOps.voxelWeighting testMod 1 1.0 -2 0 off on node:testNode
            max create mode
            max modify mode
            
            skinOps.voxelWeighting testMod 1 1.0 -1 0 off on node:testNode
            max create mode
            max modify mode
            	
            
            format "heatmap autonub		\n"
            skinOps.voxelWeighting testMod 1 1.0 -1 0 off off node:testNode
            max create mode
            max modify mode
            
            format "voxel falloff		\n"
            skinOps.voxelWeighting testMod 0 0.0 -1 64 off on node:testNode
            max create mode
            max modify mode
            
            skinOps.voxelWeighting testMod 0 1.0 -1 64 off on node:testNode
            max create mode
            max modify mode
            
            format "voxel density		\n"
            skinOps.voxelWeighting testMod 0 0.5 -1 0 off on node:testNode
            max create mode
            max modify mode
            
            skinOps.voxelWeighting testMod 0 0.5 -1 64 off on node:testNode
            max create mode
            max modify mode
            
            skinOps.voxelWeighting testMod 0 0.5 -1 128 off on node:testNode
            max create mode
            max modify mode
            	
            format "voxel winding		\n"
            skinOps.voxelWeighting testMod 0 0.5 1 64 on on node:testNode
		)
		else
		(
			format "Skipping VoxelHeatMap test because feature depends on GPU presence; no GPU found\n"
		)
	),


	/* Fixes issue when saving while editing DQ weights */
	
	function skin_MAXX_33477 =
	(
        Cylinder smooth:on heightsegs:24 capsegs:1 sides:18 height:180.925 radius:11.8185 mapcoords:on pos:[0.398957,1.51602,0] isSelected:on
		Box lengthsegs:1 widthsegs:1 heightsegs:1 length:10 width:10 height:50 mapcoords:on
		max tool animmode
		set animate off

		Box lengthsegs:1 widthsegs:1 heightsegs:1 length:11.5901 width:10.471 height:52.5281 mapcoords:on pos:[-19.6352,-1.91157,0] isSelected:on
		Box lengthsegs:1 widthsegs:1 heightsegs:1 length:13.08 width:8.72368 height:57.0647 mapcoords:on pos:[-32.0862,-1.04684,0] isSelected:on
		max move
		clearSelection()
		select $Box002
		$.pos = [-19.6352,-1.91157,68.6842]
		$.pos = [1.02133,-1.91157,57.6354]
		select $Box003
		$.pos = [0.295071,-1.04684,113.328]

		select $Box003
		$.parent = $Box002
		select $Box002
		$.parent = $Box001

		select $Box001
		selectMore $Box002
		selectMore $Box003

		sliderTime = 30f
		max tool animmode
		set animate on
		clearSelection()
		select $Box002
		rotate $ (angleaxis -63.458 [0,1,0])
		select $Box003
		rotate $ (angleaxis -63.458 [0,1,0])
		set animate off
		sliderTime = 1f
		select $Cylinder001
		max modify mode
		modPanel.addModToSelection (Skin ()) ui:on
						
							  
				  
		skinOps.addBone $.modifiers[#Skin] $Box001 1
		skinOps.addBone $.modifiers[#Skin] $Box002 1
		skinOps.addBone $.modifiers[#Skin] $Box003 1
		$.modifiers[#Skin].cross_radius = 17.1675
		sliderTime = 46f
		sliderTime = 0f
		subobjectLevel = 1
		$.modifiers[#Skin].filter_vertices = on
		skinOps.SelectVertices $.modifiers[#Skin] #{1..452}
		$.modifiers[#Skin].showNoEnvelopes = on
		sliderTime = 30f
		skinOps.enableDQOverrideWeighting $.modifiers[#Skin] true
		skinOps.SelectVertices $.modifiers[#Skin] #{110..271, 273..289, 294..296, 302..304}
		skinOps.setWeight $.modifiers[#Skin] 1
		p = getVert $ 443
		holdMaxFile() 
		fetchMaxFile quiet:true
		sliderTime = 30f
		select $Cylinder001
		p1 = getVert $ 443
		d = Length (p1-p)
		assert_true (d < 0.0001)
	),
	
	function skin_getVertexBoneIDsAndWeights =
	(
        c = Cylinder smooth:on heightsegs:24 capsegs:1 sides:18 height:180.925 radius:11.8185 pos:[0.398957,1.51602,0] 
		b1 = Box lengthsegs:1 widthsegs:1 heightsegs:1 length:10 width:10 height:50 mapcoords:on
		b2 = Box lengthsegs:1 widthsegs:1 heightsegs:1 length:11.5901 width:10.471 height:52.5281 pos:[1.02133,-1.91157,57.6354] parent:b1
		b3 = Box lengthsegs:1 widthsegs:1 heightsegs:1 length:13.08 width:8.72368 height:57.0647 pos:[0.295071,-1.04684,113.328] parent:b2

		at time 30 with animate on
		(
			rotate b2 (angleaxis -63.458 [0,1,0])
			rotate b3 (angleaxis -63.458 [0,1,0])
		)

		select $Cylinder001
		max modify mode
		skinMod = Skin ()
		modPanel.addModToSelection skinMod ui:on
				  
		skinOps.addBone skinMod $Box001 1
		skinOps.addBone skinMod $Box002 1
		skinOps.addBone skinMod $Box003 1
		skinMod.cross_radius = 17.1675
		subobjectLevel = 1
		skinMod.filter_vertices = on
		skinOps.SelectVertices skinMod #{1..452}
		skinMod.showNoEnvelopes = on
		sliderTime = 30f
		skinOps.enableDQOverrideWeighting skinMod true
		skinOps.SelectVertices skinMod #{110..271, 273..289, 294..296, 302..304}
		skinOps.setWeight skinMod 1

		holdMaxFile() 
		fetchMaxFile quiet:true
		
		select $Cylinder001
		
		skinMod = $Cylinder001.modifiers[#Skin]
		nverts=skinops.getNumberVertices skinMod
		for i = 1 to nverts do 
		(
			getVertexBoneIDsAndWeights_res = skinops.getVertexBoneIDsAndWeights skinMod i 
			assert_equal 2 getVertexBoneIDsAndWeights_res.count
			boneIDs = getVertexBoneIDsAndWeights_res[1]
			vertexWeights = getVertexBoneIDsAndWeights_res[2]
			weightCount = skinops.getVertexWeightCount skinMod i
			assert_equal weightCount boneIDs.count
			assert_equal weightCount vertexWeights.count
			for j = 1 to weightCount do
			(
				boneID = skinOps.GetVertexWeightBoneID skinMod i j
				assert_equal boneID (boneIDs[j])
				weight = skinops.getVertexWeight skinMod i j
				assert_float weight (vertexWeights[j])
			)
			skinOps.SetVertexWeights skinMod i boneIDs vertexWeights
			for j = 1 to weightCount do
			(
				boneID = skinOps.GetVertexWeightBoneID skinMod i j
				assert_equal boneID (boneIDs[j])
				weight = skinops.getVertexWeight skinMod i j
				assert_float weight (vertexWeights[j])
			)
		)
	),
/*
	MAXX_52825:
	SkinOps.addBone function does not filter the type of objects that can controll vertices, also it
	does not prevent adding an already bone to the list of bones affecting the object.
*/	
	function skin_addBoneCreatesDuplicates_MAXX_52825 =
	(
		myWeights1 = #(0.25, 0.25, 0.25, 0.25, 0.5, 0.5, 0.5, 0.5)
		myWeights2 = #(0.75, 0.75, 0.75, 0.75, 0.5, 0.5, 0.5, 0.5)

		myBox = Box length:40 height:50 width: 80
		convertToMesh myBox
		myBone1 = Dummy position:[45,0,25] size:60
		myBone2 = Dummy position:[-45,0,25] size:60

		mySkin = Skin()
		addModifier myBox mySkin
		max modify mode
		modPanel.setCurrentObject mySkin

		--add a bones
		skinOps.addBone mySkin myBone1 0
		skinOps.addBone mySkin myBone2 1
		-- try to add an added bone for the second time
		skinOps.addBone mySkin myBone2 1

		for v in 1 to myBox.numVerts do
		(
			skinOps.unNormalizeVertex mySkin v true
		)
		for v in 1 to myBox.numVerts do
		(
			skinOps.ReplaceVertexWeights mySkin v #(1, 2) #(myWeights1[v], myWeights2[v])
		)
		totalBones = skinOps.GetNumberBones myskin
		assert_equal 2 totalBones message:"The same bone is added more than once!"
	),
	
	fn rayMeshGridIntersect_MAXX_77132 =
	(
		local filePath = getFileNamePath(getSourceFileName())
		local fileName = filePath + "./TestData/intersectRayBug_2015.max"
		assert_true (loadMaxFile fileName quiet:true useFileUnits:true)

		local car = getNodeByName "body_a"

		local voxelGrid = rayMeshGridIntersect()
		voxelGrid.Initialize 88
		voxelGrid.addNode car
		voxelGrid.BuildGrid()

		local leftBound = 25
		local rightBound = -25
		local lowerBound = 0
		local upperBound = 40
		local step = 1.
		local rayDirection = point3 0 -1 0
		local nHits = 0

		for x = rightBound to leftBound by step do
		(
			for z = lowerBound to upperBound by step do
			(
				startPoint = point3 x 70 z

				if (voxelGrid.intersectRay startPoint rayDirection false == 0) then
				(
					box position:startPoint height:(abs step) width:(abs step) length:(abs step) wirecolor:green
				)
				else
				(
					box position:startPoint height:(abs step) width:(abs step) length:(abs step) wirecolor:red
					nHits += 1
				)
			)
		)
		voxelGrid.free()
		assert_equal 870 nHits
	),
	
	Tests =
	#(
		skin_MAXX_11350, 
		skin_DQ_UnitTest, 
		skin_VoxelHeatMap_UnitTest, 
		skin_MAXX_33477,
		skin_getVertexBoneIDsAndWeights,
		skin_addBoneCreatesDuplicates_MAXX_52825,
		rayMeshGridIntersect_MAXX_77132
	)
)

run_test_fixture Obj_Prop_UnitTest script:(getThisScriptFilename())