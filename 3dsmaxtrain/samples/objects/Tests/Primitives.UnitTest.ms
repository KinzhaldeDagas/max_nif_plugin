
--------------------------------------------------------------------------------
-- Tests for primitives
-- Author:	Tom Hudson
-- Created:	January 2022
--------------------------------------------------------------------------------

Struct  PrimitiveUnitTests
(
	Function setup =
	(
		resetmaxFile #noPrompt
	),
	Function teardown =
	(
	),

	Function primitiveTests =
	(
		format "Primitive Test 1a: Default Sphere\n"
		
		s = Sphere radius:18.6358 pos:[5.81073,-11.4021,0] isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 114 verts message:"Sphere default vertex count error"
		assert_equal 240 edges message:"Sphere default edge count error"
		assert_equal 128 faces message:"Sphere default face count error"
		delete s
		
		format "Primitive Test 1b: Minimum Sphere\n"
		
		s = Sphere radius:18.6358 pos:[5.81073,-11.4021,0] segs:0 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 6 verts message:"Sphere minimum vertex count error"
		assert_equal 12 edges message:"Sphere minimum edge count error"
		assert_equal 8 faces message:"Sphere minimum face count error"
		delete s
		
		format "Primitive Test 1c: Maximum Sphere\n"
		
		s = Sphere radius:18.6358 pos:[5.81073,-11.4021,0] segs:9999 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 6476402 verts message:"Sphere maximum vertex count error"
		assert_equal 12956400 edges message:"Sphere maximum edge count error"
		assert_equal 6480000 faces message:"Sphere maximum face count error"
		delete s

		format "Primitive Test 2a: Default Torus\n"
		
		t = Torus pos:[-2.05586,1.62306,0] isSelected:on radius1:26.8941 radius2:10.9784
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 288 verts message:"Torus default vertex count error"
		assert_equal 576 edges message:"Torus default edge count error"
		assert_equal 288 faces message:"Torus default face count error"
		delete t
		
		format "Primitive Test 2b: Minimum Torus\n"
		
		t = Torus pos:[-2.05586,1.62306,0] isSelected:on radius1:26.8941 radius2:10.9784 segs:0 sides:0
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 9 verts message:"Torus minimum vertex count error"
		assert_equal 18 edges message:"Torus minimum edge count error"
		assert_equal 9 faces message:"Torus minimum face count error"
		delete t
		
		format "Primitive Test 2c: Maximum Torus\n"
		
		t = Torus pos:[-2.05586,1.62306,0] isSelected:on radius1:26.8941 radius2:10.9784 segs:9999 sides:9999
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 12960000 verts message:"Torus maximum vertex count error"
		assert_equal 25920000 edges message:"Torus maximum edge count error"
		assert_equal 12960000 faces message:"Torus maximum face count error"
		delete t

		format "Primitive Test 3a: Default Cone\n"
		
		c = Cone radius1:15.2474 radius2:8.71656 height:28.6354 pos:[-69.9941,14.8983,0] isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 144 verts message:"Cone default vertex count error"
		assert_equal 264 edges message:"Cone default edge count error"
		assert_equal 122 faces message:"Cone default face count error"
		delete c
		
		format "Primitive Test 3b: Minimum Cone\n"
		
		c = Cone radius1:15.2474 radius2:8.71656 height:28.6354 pos:[-69.9941,14.8983,0] heightsegs:0 capsegs:0 sides:0 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 6 verts message:"Cone minimum vertex count error"
		assert_equal 9 edges message:"Cone minimum edge count error"
		assert_equal 5 faces message:"Cone minimum face count error"
		delete c
		
		format "Primitive Test 3c: Maximum Cone\n"
		
		c = Cone radius1:15.2474 radius2:8.71656 height:28.6354 pos:[-69.9941,14.8983,0] heightsegs:9999 capsegs:9999 sides:9999 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 10796402 verts message:"Cone maximum vertex count error"
		assert_equal 21596400 edges message:"Cone maximum edge count error"
		assert_equal 10800000 faces message:"Cone maximum face count error"
		delete c

		format "Primitive Test 4a: Default Cylinder\n"
		
		c = Cylinder radius:21.6641 height:18.8391 pos:[10.5624,-53.5873,0] isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 48 verts message:"Cylinder default vertex count error"
		assert_equal 72 edges message:"Cylinder default edge count error"
		assert_equal 26 faces message:"Cylinder default face count error"
		delete c
		
		format "Primitive Test 4b: Minimum Cylinder\n"
		
		c = Cylinder radius:21.6641 height:18.8391 pos:[10.5624,-53.5873,0] heightsegs:0 capsegs:0 sides:0 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 6 verts message:"Cylinder minimum vertex count error"
		assert_equal 9 edges message:"Cylinder minimum edge count error"
		assert_equal 5 faces message:"Cylinder minimum face count error"
		delete c
		
		format "Primitive Test 4c: Maximum Cylinder\n"
		
		c = Cylinder radius:21.6641 height:18.8391 pos:[10.5624,-53.5873,0] heightsegs:9999 capsegs:9999 sides:9999 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 10796402 verts message:"Cylinder maximum vertex count error"
		assert_equal 21596400 edges message:"Cylinder maximum edge count error"
		assert_equal 10800000 faces message:"Cylinder maximum face count error"
		delete c

		format "Primitive Test 5a: Default Tube\n"
		
		t = Tube radius1:11.0577 radius2:5.62157 height:29.2836 pos:[-72.6151,-35.8701,0] isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 96 verts message:"Tube default vertex count error"
		assert_equal 192 edges message:"Tube default edge count error"
		assert_equal 96 faces message:"Tube default face count error"
		delete t
		
		format "Primitive Test 5b: Minimum Tube\n"
		
		t = Tube radius1:11.0577 radius2:5.62157 height:29.2836 pos:[-72.6151,-35.8701,0] heightsegs:0 capsegs:0 sides:0 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 12 verts message:"Tube minimum vertex count error"
		assert_equal 24 edges message:"Tube minimum edge count error"
		assert_equal 12 faces message:"Tube minimum face count error"
		delete t
		
		format "Primitive Test 5c: Maximum Tube\n"
		
		t = Tube radius1:11.0577 radius2:5.62157 height:29.2836 pos:[-72.6151,-35.8701,0] heightsegs:9999 capsegs:9999 sides:9999 isSelected:on
		convertTo $ PolyMeshObject
		verts = $.numVerts
		edges = $.GetNumEdges()
		faces = $.numFaces
		assert_equal 14400000 verts message:"Tube maximum vertex count error"
		assert_equal 28800000 edges message:"Tube maximum edge count error"
		assert_equal 14400000 faces message:"Tube maximum face count error"
		delete t

	),

	Tests  = #(
		primitiveTests
		) 
)

if undefined == run_test_fixture then
(
	format "-- No fixture!\n" 

	foo = PrimitiveUnitTests  () 
	for test in foo.Tests do
		(
		foo.setup()
		test()
		foo.teardown()
		)
) 
else 
( 
	format "-- Fixture found!\n" 
	run_test_fixture  PrimitiveUnitTests  script:(getThisScriptFilename()) 
) 
