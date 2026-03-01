fileIn "MxsUnit/MxsUnit.ms"

struct LayerControl_TestFixture
(

	fn setUp =
	(
		resetMaxFile #noprompt 
	),
	
	fn tearDown =
	(
	),
	
	fn test_dead_layer_removal = 
	(
		print "running test_dead_layer_removal"

		b1= Box name:"B1"
		AnimLayerManager.enableLayers b1
		b2= Box name:"B2"
		AnimLayerManager.enableLayers b2
		AnimLayerManager.addLayer "B2" b2 true
		assert_equal 2 (AnimLayerManager.getLayerCount())
		delete b2
		holdMaxFile()
		fetchMaxFile quiet:true
		assert_equal 1 (AnimLayerManager.getLayerCount())
		assert_string_equal "Base Layer" (AnimLayerManager.getLayerName 1)
	),

	fn test_node_removal = 
	(
		resetMaxFile quiet:true
		print "running test_node_removal"

		wasHolding = theHold.Holding()
		if wasHolding do
			theHold.Accept "test_node_removal"
		
		b1= Box name:"B1"
		b2= Box name:"B2"
		AnimLayerManager.enableLayers #(b1,b2)
		
		AnimLayer = trackViewNodes[#Anim_Layer_Control_Manager].object.AnimLayers[1]
		assert_equal 9 (refs.getNumRefs AnimLayer)
		with undo on ( delete b1)
		assert_equal 8 (refs.getNumRefs AnimLayer)
		max undo
		assert_equal 9 (refs.getNumRefs AnimLayer)
		max redo
		assert_equal 8 (refs.getNumRefs AnimLayer)
		holdMaxFile()
		clearundobuffer()
		assert_equal 5 (refs.getNumRefs AnimLayer)
		assert_equal $b2.pos.controller (refs.getReference AnimLayer 2)
		assert_equal $b2.rotation.controller (refs.getReference AnimLayer 3)
		assert_equal $b2.scale.controller (refs.getReference AnimLayer 4)
		assert_equal $b2 (refs.getReference AnimLayer 5).node
		fetchMaxFile quiet:true
		AnimLayer = trackViewNodes[#Anim_Layer_Control_Manager].object.AnimLayers[1]
		assert_equal 5 (refs.getNumRefs AnimLayer)
		assert_equal $b2.pos.controller (refs.getReference AnimLayer 2)
		assert_equal $b2.rotation.controller (refs.getReference AnimLayer 3)
		assert_equal $b2.scale.controller (refs.getReference AnimLayer 4)
		assert_equal $b2 (refs.getReference AnimLayer 5).node
		
		resetMaxFile quiet:true
		
		b1= Box name:"B1"
		b2= Box name:"B2"
		AnimLayerManager.enableLayers #(b1,b2)
		
		AnimLayer = trackViewNodes[#Anim_Layer_Control_Manager].object.AnimLayers[1]
		assert_equal 9 (refs.getNumRefs AnimLayer)
		with undo on ( delete b2)
		assert_equal 8 (refs.getNumRefs AnimLayer)
		max undo
		assert_equal 9 (refs.getNumRefs AnimLayer)
		max redo
		assert_equal 8 (refs.getNumRefs AnimLayer)
		holdMaxFile()
		clearundobuffer()
		assert_equal 5 (refs.getNumRefs AnimLayer)
		assert_equal $b1.pos.controller (refs.getReference AnimLayer 2)
		assert_equal $b1.rotation.controller (refs.getReference AnimLayer 3)
		assert_equal $b1.scale.controller (refs.getReference AnimLayer 4)
		assert_equal $b1 (refs.getReference AnimLayer 5).node
		fetchMaxFile quiet:true
		AnimLayer = trackViewNodes[#Anim_Layer_Control_Manager].object.AnimLayers[1]
		assert_equal 5 (refs.getNumRefs AnimLayer)
		assert_equal $b1.pos.controller (refs.getReference AnimLayer 2)
		assert_equal $b1.rotation.controller (refs.getReference AnimLayer 3)
		assert_equal $b1.scale.controller (refs.getReference AnimLayer 4)
		assert_equal $b1 (refs.getReference AnimLayer 5).node
		
		if wasHolding do 
			theHold.Begin()
	),

	tests = 
	#(
		test_dead_layer_removal,
		test_node_removal
	)
		
)

run_test_fixture LayerControl_TestFixture script:(getThisScriptFilename())
