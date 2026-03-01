/*
    @param TestName:        MXSAgni.Misc.UnitTest
    @param Author:          larry.minton@autodesk.com
    @param Created:         12-06-2019 
    @param LastMod:         04-05-2024
    @param ModBy:           Larry Minton
    @param Description:     Miscelleous MXSAgni tests
    @param Status:          Online
*/

fileIn "MxsUnit/MxsUnit.ms"

-- set things up so can run tests multiple times w/o error
global mxsagni_misc_Tests_bad_cid;
if mxsagni_misc_Tests_bad_cid == undefined do
	mxsagni_misc_Tests_bad_cid= #(0xdeadL, 0xbeefL)

struct mxsagni_misc_Tests
(
	fn test_sysinfo_MAXpriority_enum_values = 
	(
		print "running test_sysinfo_MAXpriority_enum_values"
		print "backgroundBegin tweak 3"
		local current_priority = sysinfo.MAXpriority
		format "initial sysinfo.MAXpriority: %\n" current_priority
		local read_write_priorities = #(#low,#belowNormal,#normal,#aboveNormal,#high)
		for p in read_write_priorities do 
		(	
			sysinfo.MAXpriority = p
			assert_equal p sysinfo.MAXpriority
		)
		-- special case - setting #backgroundBegin doesn't change reported priority in win7, but does set priority to #low in win10, and to
		-- #normal (at least some cases) in win11
		for p in read_write_priorities do 
		(	
			format "testing priority %\n" p
			sysinfo.MAXpriority = p;
			local priority = sysinfo.MAXpriority
			format "testing - return value changing? % : % : %\n" priority sysinfo.MAXpriority sysinfo.MAXpriority
			assert_equal p priority message:("p: #" + p + "; sysinfo.MAXpriority: #" + priority)
			sysinfo.MAXpriority = #backgroundBegin
			priority = sysinfo.MAXpriority
			format "testing #backgroundBegin- return value changing? % : % : %\n" priority sysinfo.MAXpriority sysinfo.MAXpriority
			assert_true (priority == p or priority == #low or priority == #normal) message:("p: #" + p + "; sysinfo.MAXpriority: #" + priority)
			sysinfo.MAXpriority = #backgroundEnd
			priority = sysinfo.MAXpriority
			format "testing #backgroundEnd- return value changing? % : % : %\n" priority sysinfo.MAXpriority sysinfo.MAXpriority
			assert_true (priority == p or priority == #low or priority == #normal) message:("p: #" + p + "; sysinfo.MAXpriority: #" + priority)
		)
		-- special case - setting realtime priority sets priority to high if not running with admin priv. 
		sysinfo.MAXpriority = #realtime
		local priority = sysinfo.MAXpriority
		assert_true (priority == #realtime or priority == #high) message:("sysinfo.MAXpriority: #" + priority)
		sysinfo.MAXpriority = current_priority
		format "final sysinfo.MAXpriority: %\n" sysinfo.MAXpriority
	),
	
	fn test_get_set_ini =
	(
		print "running test_get_set_ini"
		local TEST_INI_FILE = @"$temp\get_set_ini_test_file.ini"
		deletefile TEST_INI_FILE
		local long_line = "XXXXXXXXXXXXXXXX"
		do long_line += long_line while long_line.count < 16384
		local res = setIniSetting TEST_INI_FILE "section" "key" long_line
		assert_true res
		local read_long_line = getINISetting TEST_INI_FILE "section" "key"
		assert_equal long_line.count read_long_line.count
		long_line += substring long_line 2 -1
		res = setIniSetting TEST_INI_FILE "section" "key" long_line
		assert_true res
		read_long_line = getINISetting TEST_INI_FILE "section" "key"
		assert_equal long_line.count read_long_line.count
		deletefile TEST_INI_FILE
		-- following will write / attempt to read a 32k key value. 
		long_line += "A"
		res = setIniSetting TEST_INI_FILE "section" "key" long_line
		assert_true res
		read_long_line = getINISetting TEST_INI_FILE "section" "key"
		-- due to GetPrivateProfileString behavior, we will get back a zero length string, usually....
		if read_long_line.count != long_line.count do
			assert_equal 0 read_long_line.count
	),
	
	fn test_SingleRefMaker_TypedSingleRefMaker =
	(
		print "running test_SingleRefMaker_TypedSingleRefMaker"
		local res = ValidateSingleRefMaker()
		assert_true res
	),

	fn test_used_in_scene =
	(
		print "running test_used_in_scene"
		resetmaxfile #noprompt
		local t1 = teapot()
		local m1=standardmaterial()
		assert_false (isMtlUsedInSceneMtl m1)
		assert_false (isUsedInScene m1)
		t1.material = m1
		assert_true (isMtlUsedInSceneMtl m1)
		assert_true (isUsedInScene m1)
		m1.diffusemap = dent()
		assert_true (isMtlUsedInSceneMtl m1.diffusemap)
		assert_true (isUsedInScene m1.diffusemap)
		local t2 = teapot()
		local m2=multimaterial()
		t2.material = m2
		assert_true (isMtlUsedInSceneMtl m2)
		assert_true (isUsedInScene m2)
		if m2[1] == undefined then m2[1] = physicalmaterial()
		local subMtl = m2[1]
		assert_true (isMtlUsedInSceneMtl subMtl)
		assert_true (isUsedInScene subMtl)
		assert_false (isMtlUsedInSceneMtl subMtl topLevelMtlOnly:true)
		test_used_in_scene_ca= attributes test_used_in_scene_ca_def
		(
			parameters main 
			(
				myMtl type:#material 
				myTexMap type:#texturemap  
			)
		)
		custattributes.add t1 test_used_in_scene_ca
		local m3=standardmaterial()
		local tmap1 = dent()
		t1.myMtl = m3
		t1.myTexMap = tmap1
		assert_false (isMtlUsedInSceneMtl m3)
		assert_true (isUsedInScene m3)
		assert_false (isMtlUsedInSceneMtl tmap1)
		assert_true (isUsedInScene tmap1)
		
		assert_false (isMtlUsedInSceneMtl m3 skipCustAttributes:true)
		assert_false (isUsedInScene m3 skipCustAttributes:true)
		assert_false (isMtlUsedInSceneMtl tmap1 skipCustAttributes:true)
		assert_false (isUsedInScene tmap1 skipCustAttributes:true)
		
		assert_false (isMtlUsedInSceneMtl m3 skipCustAttributes:false)
		assert_true (isUsedInScene m3 skipCustAttributes:false)
		assert_false (isMtlUsedInSceneMtl tmap1 skipCustAttributes:false)
		assert_true (isUsedInScene tmap1 skipCustAttributes:false)
		
		custattributes.add m1 test_used_in_scene_ca
		m1.myMtl = m3
		m1.myTexMap = tmap1
		assert_false (isMtlUsedInSceneMtl m3)
		assert_false (isMtlUsedInSceneMtl tmap1)
		assert_false (isMtlUsedInSceneMtl m3 skipCustAttributes:true)
		assert_false (isMtlUsedInSceneMtl tmap1 skipCustAttributes:true)
		assert_true (isMtlUsedInSceneMtl m3 skipCustAttributes:false)
		assert_true (isMtlUsedInSceneMtl tmap1 skipCustAttributes:false)
		
		saveMaxFile @"$temp\test_used_in_scene.max" quiet:true
		
		assert_equal 2 sceneMaterials.count
		
		local nodeRefs = for i = 1 to (refs.getnumrefs t2) collect (refs.getreference t2 i)
		for r in nodeRefs where r != undefined do assert_true (isUsedInScene r) 
		
		delete t2
		assert_false (isMtlUsedInSceneMtl m2)
		assert_false (isUsedInScene m2)
		
		for r in nodeRefs where r != undefined and (classof r != Base_LayerBase_Layer) do assert_false (isUsedInScene r) message:("ref: " + r as string)
		for r in nodeRefs where r != undefined and (classof r == Base_LayerBase_Layer) do assert_true (isUsedInScene r) message:("ref: " + r as string)

		assert_equal 2 sceneMaterials.count
		UpdateSceneMaterialLib()
		assert_equal 1 sceneMaterials.count
		
		resetmaxfile #noprompt
		xrefs.addNewXRefFile @"$temp\test_used_in_scene.max" 
		local xref = xrefs.getXRefFile 1
		local mtl = xref.tree.children[1].material
		assert_false (isMtlUsedInSceneMtl mtl)
		assert_false (isUsedInScene mtl)
		assert_true (isMtlUsedInSceneMtl mtl includeXrefScenes:true)
		assert_true (isUsedInScene mtl includeXrefScenes:true)
		resetmaxfile #noprompt
	),

	fn meshop_attach_test_op count = 
	(
		with undo off with redraw off
		(
			t = Teapot pos:[-200,200,0] isSelected:on radius:10 segs:64
			converttomesh t
			copies = for i = 1 to (count-1) collect (c = copy t; c.pos.x += (i*10); c)
			for n in copies do meshop.attach t n
			copies = for i = 1 to (count-1) collect (c = copy t; c.pos.y += (i*10); c)
			for n in copies do meshop.attach t n
			copies = for i = 1 to (count-1) collect (c = copy t; c.pos.z += (i*10); c)
			for n in copies do meshop.attach t n
			copies = t = n = undefined
			delete objects
			ok
		)
	),
	
	fn meshop_attach_test = 
	(
		print "running meshop_attach_test"
		meshop_attach_test_op 1 -- seed to get better memory leakage measurement below
		resetmaxfile #noprompt
		gc()
		startMem = sysinfo.getMAXHeapUsed()
		meshop_attach_test_op 4
		endMem = sysinfo.getMAXHeapUsed()
		memLeakage = (endMem - startMem)/(1024*1024)
		format "meshop_attach_test leakage: %MB\n" memLeakage
		assert_true (memLeakage < 10) message:("leakage: " + memLeakage as string + "MB")
	),

	fn meshop_polyop_get_listarg_test =
	(
		print "running meshop_polyop_get_listarg_test"
		acouple = #{1..3,6}
		
		b_mesh = box pos:[100,100,0]
		converttomesh b_mesh
		nverts = b_mesh.numVerts
		nfaces = b_mesh.numFaces
		
		verts1 = for i = 1 to nverts collect (meshop.getVert b_mesh i)
		verts2 = meshop.getVerts b_mesh #all
		assert_equal nverts verts2.count
		for i = 1 to nverts do assert_equal verts1[i] verts2[i]

		verts1 = for i in acouple collect (meshop.getVert b_mesh i)
		verts2 = meshop.getVerts b_mesh acouple
		assert_equal verts1.count verts2.count
		for i = 1 to verts1.count do assert_equal verts1[i] verts2[i]

		faces1 = for i = 1 to nfaces collect (getFace b_mesh i)
		faces2 = for i = 1 to nfaces collect (meshop.getFace b_mesh i)
		faces3 = meshop.getFaces b_mesh #all
		assert_equal nfaces faces3.count
		for i = 1 to nfaces do assert_equal faces1[i] faces2[i]
		for i = 1 to nfaces do assert_equal faces1[i] faces3[i]
		
		faces1 = for i in acouple collect (getFace b_mesh i)
		faces2 = for i in acouple collect (meshop.getFace b_mesh i)
		faces3 = meshop.getFaces b_mesh acouple
		assert_equal faces1.count faces3.count
		for i = 1 to faces1.count do assert_equal faces1[i] faces2[i]
		for i = 1 to faces1.count do assert_equal faces1[i] faces3[i]
		
		b_poly = box pos:[100,100,0]
		converttopoly b_poly 
		nfaces = polyop.getNumFaces b_poly
		nverts = polyop.getNumVerts b_poly
		nedges = polyop.getNumEdges b_poly
		
		verts1 = for i = 1 to nedges collect (polyop.getEdgeVerts b_poly i)
		verts2 = polyop.getEdgesVerts b_poly #all
		assert_equal nedges verts2.count
		for i = 1 to nedges do for j = 1 to verts1[i].count do assert_equal verts1[i][j] verts2[i][j]

		verts1 = for i in acouple collect (polyop.getEdgeVerts b_poly i)
		verts2 = polyop.getEdgesVerts b_poly acouple
		assert_equal verts1.count verts2.count
		for i = 1 to verts1.count do for j = 1 to verts1[i].count do assert_equal verts1[i][j] verts2[i][j]

		faces1 = for i = 1 to nedges collect (polyop.getEdgeFaces b_poly i)
		faces2 = polyop.getEdgesFaces b_poly #all
		assert_equal nedges faces2.count
		for i = 1 to nedges do for j = 1 to faces1[i].count do assert_equal faces1[i][j] faces2[i][j]

		faces1 = for i in acouple collect (polyop.getEdgeFaces b_poly i)
		faces2 = polyop.getEdgesFaces b_poly acouple
		assert_equal faces1.count faces2.count
		for i = 1 to faces1.count do for j = 1 to faces1[i].count do assert_equal faces1[i][j] faces2[i][j]

		verts1 = for i = 1 to nfaces collect (polyop.getFaceVerts b_poly i)
		verts2 = polyop.getFacesVerts b_poly #all
		assert_equal nfaces verts2.count
		for i = 1 to nfaces do for j = 1 to verts1[i].count do assert_equal verts1[i][j] verts2[i][j]

		verts1 = for i in acouple collect (polyop.getFaceVerts b_poly i)
		verts2 = polyop.getFacesVerts b_poly acouple
		assert_equal verts1.count verts2.count
		for i = 1 to verts1.count do for j = 1 to verts1[i].count do assert_equal verts1[i][j] verts2[i][j]

		edges1 = for i = 1 to nfaces collect (polyop.getFaceEdges b_poly i)
		edges2 = polyop.getFacesEdges b_poly #all
		assert_equal nfaces edges2.count
		for i = 1 to nfaces do for j = 1 to edges1[i].count do assert_equal edges1[i][j] edges2[i][j]

		edges1 = for i in acouple collect (polyop.getFaceEdges b_poly i)
		edges2 = polyop.getFacesEdges b_poly acouple
		assert_equal edges1.count edges2.count
		for i = 1 to edges1.count do for j = 1 to edges1[i].count do assert_equal edges1[i][j] edges2[i][j]

		verts1 = for i = 1 to nverts collect (polyop.getVert b_poly i)
		verts2 = polyop.getVerts b_poly #all
		assert_equal nverts verts2.count
		for i = 1 to nverts do assert_equal verts1[i] verts2[i]

		verts1 = for i in acouple collect (polyop.getVert b_poly i)
		verts2 = polyop.getVerts b_poly acouple
		assert_equal verts1.count verts2.count
		for i = 1 to verts1.count do assert_equal verts1[i] verts2[i]
	),
	
	fn meshop_getMatIdTests = 
	(
		print "running meshop_getMatIdTests"
		test_mesh = box pos:[150,150,0]
		converttomesh test_mesh
		nFaces = test_mesh.numFaces
		nMats = nFaces / 2
		
		multi_mat = multimaterial numsubs: nMats
		test_mesh.material = multi_mat
		
		for i = 1 to ( nMats + 1) do
		(
			mtlFace = meshop.getFacesByMatId test_mesh i
			mtlVert = meshop.getVertsByMatId test_mesh i

			if i > nMats then
			(
				assert_equal mtlFace.numberSet 0 message: "Error in test meshop_getMatIdTests (non-zero number of faces found for invalid ID)"
				assert_equal mtlVert.numberSet 0 message: "Error in test meshop_getMatIdTests (non-zero number of verts found for invalid ID)"

			)
			else
			(
				assert_equal mtlFace.numberSet 2 message: "Error in test meshop_getMatIdTests (Expected 2 faces per material.)"
				assert_equal mtlVert.numberSet 4 message: "Error in test meshop_getMatIdTests (Expected 4 verts per material.)"
				for faceId in mtlFace do 
				( 
					assert_equal i (getFaceMatId test_mesh faceId) message: "Error in test meshop_getMatIdTests (Incorrect FaceId was retrieved from getFacesBymatId)"
					testFace = getFace test_mesh faceId
					for j = 1  to 3  do 
					(
						found = findItem mtlVert testFace[j]
						assert_true (found > 0)	message: "Error in test meshop_getMatIdTests (Expected vert was not found)"
					)
				)
			)	
		)

		--test meshop getfacematId, getfacesmatid, setfacematid
		for i = 1 to nFaces do 
		(
			newMatId =  mod i nFaces
			meshop.setfacematid test_mesh i (newMatId + 1)
		)

		for i = 1 to nFaces do 
		(
			expectedMatId = (mod i nFaces) + 1
			matId =  meshop.getfacematid test_mesh i 
			assert_equal expectedMatId matId message:("Error in test meshop_getMatIdTests getFaceMatId receives incorrect matid for face " + (i as String))
		)

		specificFaces = #(1,3,6)
		specificMatIds = meshop.getfacesmatid test_mesh (specificFaces as BitArray)

		for i = 1 to specificFaces.Count do 
		(
			expectedMatId = (mod specificFaces[i] nFaces) + 1
			matId =  specificMatIds[i]
			assert_equal expectedMatId matId message:("Error in test meshop_getMatIdTests getFacesMatId receives incorrect matid from specific face list for face " + (i as String))
		)
	),

	fn polyop_getMatIdTests = 
	(
		print "running polyop_getMatIdTests"
		test_poly = box pos:[150,150,0]
		converttopoly test_poly
		nFaces = test_poly.numFaces
		multi_mat = multimaterial numsubs: nFaces
		test_poly.material = multi_mat
		for i = 1 to ( nFaces + 1) do
		(
			mtlFace = polyop.getFacesByMatId test_poly i
			mtlVert = polyop.getVertsByMatId test_poly i
		
			if i > nFaces then
			(
				assert_equal 0 mtlFace.numberSet message: "Error in test polyop_getMatIdTests (non-zero number of faces found for invalid ID)"
				assert_equal 0 mtlVert.numberSet message: "Error in test polyop_getMatIdTests (non-zero number of verts found for invalid ID)"

			)
			else
			(
				assert_equal 1 mtlFace.numberSet message: "Error in test polyop_getMatIdTests (Expected 1 face per material.)"
				assert_equal 4 mtlVert.numberSet message: "Error in test polyop_getMatIdTests (Expected 4 verts per material.)"
				for faceId in mtlFace do 
				( 
					assert_equal i (polyop.getFaceMatID test_poly faceId) message: "Error in test polyop_getMatIdTests (Incorrect FaceId was retrieved from getFacesBymatId)"
					expectedVerts = polyop.getFaceVerts test_poly faceId
					for j = 1  to (expectedVerts.count)  do 
					(
						found = findItem mtlVert expectedVerts[j]
						assert_true (found > 0) message: "Error in test polyop_getMatIdTests (Expected vert was not found)"
					)
				)
			)	
		)

		for i = 1 to nFaces do 
		(
			newMatId =  mod i nFaces
			polyop.setfacematid test_poly i (newMatId + 1)
		)

		for i = 1 to nFaces do 
		(
			expectedMatId = (mod i nFaces) + 1
			matId =  polyop.getfacematid test_poly i 
			assert_equal expectedMatId matId message:("Error in test polyop_getMatIdTests getFaceMatId receives incorrect matid for face " + (i as String))
		)

		specificFaces = #(1,3,6)
		specificMatIds = polyop.getfacesmatid test_poly (specificFaces as BitArray)

		for i = 1 to specificFaces.Count do 
		(
			expectedMatId = (mod specificFaces[i] nFaces) + 1
			matId =  specificMatIds[i]
			assert_equal expectedMatId matId message:("Error in test polyop_getMatIdTests getFacesMatId receives incorrect matid from specific face list for face " + (i as String))
		)

		polyop.setfacematid test_poly #all 1
		polyop.setFaceFlags test_poly 3 2
		polyop.setFaceFlags test_poly 6 2
		matIdList = polyop.getfacesmatid test_poly #all
		assert_equal nFaces matIdList.count message:"Error in test polyop_getMatIdTests getfacesmatid does not return enough faces"
		for i = 1 to matIdList.count do 
		(
			if mod i 3 ==0 	then 
			(
				assert_undefined matIdList[i] message:("Error in test polyop_getMatIdTests getFacesmatids does not return the correct matid for face " + (i as String))
			)
			else 
			(
				assert_equal 1 matIdList[i] message:("Error in test polyop_getMatIdTests getFacesmatids does not return the correct matid for face " + (i as String))

			)
		)

	),

	fn test_getFaces = 
	(
		print "running test_getFaces"
		test_obj = sphere segments:100
		meshExpectedFaceCount = 9800
		polyExpectedFaceCount = 5000

		converttomesh test_obj
		faceBitArray = meshop.getallfaces test_obj
		numFaces =  faceBitArray.numberSet
		assert_equal numFaces meshExpectedFaceCount message: "Error in test test_getFaces : number of faces in mesh doesn't match what we expected"
		
		converttopoly test_obj
		polyop.setFaceFlags test_obj 16 2 -- 2 is the value of a dead face so this sets some faces(16,8,4) to dead 
		polyop.setFaceFlags test_obj 8 2 
		polyop.setFaceFlags test_obj 4 2 

		faceBitArray =  polyop.getallfaces test_obj
		deadfaceArray = polyop.getdeadfaces test_obj
		numFaces =  faceBitArray.numberSet + deadfaceArray.numberset
		assert_equal numFaces polyExpectedFaceCount message: "Error in test test_getFaces : number of faces in poly doesn't match what we expected"

		delete test_obj

	),
	
	fn getLogFileFileNumLines =
	(
		local nlines = 0;
		local f = openfile (logsystem.getNetLogFileName())
		try (while not (eof f) do (readline f; nlines +=1))
		catch ()
		close f
		nlines
	),

	fn test_systemTools_sceneIODebugEnabled =
	(
		print "running test_systemTools_sceneIODebugEnabled"
		local sceneIODebugEnabled = systemTools.sceneIODebugEnabled
		resetmaxfile #noprompt
		systemTools.sceneIODebugEnabled = false;
		local longevity = logsystem.longevity
		logsystem.longevity = #forever
		local nlines1 = getLogFileFileNumLines()
		holdmaxfile quiet:true
		local nlines2 = getLogFileFileNumLines()
		systemTools.sceneIODebugEnabled = true;
		holdmaxfile quiet:true
		local nlines3 = getLogFileFileNumLines()
		systemTools.sceneIODebugEnabled = sceneIODebugEnabled;
		logsystem.longevity = longevity
		format "test_systemTools_sceneIODebugEnabled : % : %\n" (nlines2 - nlines1) (nlines3 - nlines2)
		assert_true ((nlines3 - nlines2) > (nlines2 - nlines1 + 300))
	),

	fn test_getMAXClass =
	(
		print "running test_getMAXClass"
		assert_equal box (getMAXClass box.superclassid box.classid)
		mxsagni_misc_Tests_bad_cid[2] += 1
		assert_undefined (getMAXClass box.superclassid mxsagni_misc_Tests_bad_cid)
		local c = (getMAXClass box.superclassid mxsagni_misc_Tests_bad_cid create:true)
		assert_true (iskindof c maxclass)
		assert_false (c.creatable)
	),

	fn test_persistents =
	(
		print "running test_persistents"
		persistent_globals = persistents.removeAll()
		persistent_globals = persistents.gather()
		assert_equal 0 persistent_globals.count

		execute "global my_non_persistent_global = 10"
		assert_true (globalVars.isGlobal #my_non_persistent_global)
		assert_false (persistents.isPersistent #my_non_persistent_global)
		globalVars.remove #my_non_persistent_global
		assert_false (globalVars.isGlobal #my_non_persistent_global)
		assert_false (persistents.isPersistent #my_non_persistent_global)
		
		--same test but by passing strings
		execute "global my_non_persistent_global = 10"
		assert_true (globalVars.isGlobal "my_non_persistent_global")
		assert_false (persistents.isPersistent "my_non_persistent_global")
		globalVars.remove "my_non_persistent_global"
		assert_false (globalVars.isGlobal "my_non_persistent_global")
		assert_false (persistents.isPersistent "my_non_persistent_global")

		execute "persistent global my_persistent_global = 10"
		persistent_globals = persistents.gather()
		assert_equal 1 persistent_globals.count
		assert_equal #my_persistent_global persistent_globals[1]
		assert_true (globalVars.isGlobal #my_persistent_global)
		assert_true (persistents.isPersistent #my_persistent_global)
		assert_true (persistents.isPersistent "my_persistent_global")
		
		ss = stringStream ""
		persistents.show to:ss
		seek ss 0
		l = readline ss
		assert_equal #my_persistent_global ((filterstring l ":")[1] as name)
		persistents.remove #my_persistent_global
		assert_true (globalVars.isGlobal #my_persistent_global)
		assert_false (persistents.isPersistent #my_persistent_global)
		persistents.make #my_persistent_global
		assert_true (globalVars.isGlobal #my_persistent_global)
		assert_true (persistents.isPersistent #my_persistent_global)
		
		--same test but by passing strings
		persistents.remove "my_persistent_global"
		assert_true (globalVars.isGlobal "my_persistent_global")
		assert_false (persistents.isPersistent "my_persistent_global")
		persistents.make "my_persistent_global"
		assert_true (globalVars.isGlobal "my_persistent_global")
		assert_true (persistents.isPersistent "my_persistent_global")
		
		globalVars.remove #my_persistent_global
		assert_false (globalVars.isGlobal #my_persistent_global)
		assert_false (persistents.isPersistent #my_persistent_global)
		assert_false (persistents.isPersistent "my_persistent_global")
	),
	
	fn test_memStreamMgr_eos =
	(
		print "running test_memStreamMgr_eos"
		s = "Hello World"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_string_equal (f.readline()) ""
		assert_string_equal (f.readline()) ""
		assert_string_equal (f.readline()) ""
		assert_string_equal (f.readline()) ""
		memStreamMgr.close f

		s = "Hello World\r"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		memStreamMgr.close f

		s = "Hello World\r\n"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		memStreamMgr.close f

		s = "Hello World\rHello World\r"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		memStreamMgr.close f

		s = "Hello World\r\nHello World\r\n"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		memStreamMgr.close f

		s = "Hello World\r\r"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.readline()) "Hello World"
		assert_false (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		assert_string_equal (f.readline()) ""
		assert_true (f.eos())
		memStreamMgr.close f

		s = "H"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.peekchar()) "H"
		assert_string_equal (f.readchar()) "H"
		assert_true (f.eos())
		assert_string_equal (f.peekchar()) ""
		assert_string_equal (f.readchar()) ""
		assert_true (f.eos())
		assert_string_equal (f.peekchar()) ""
		assert_string_equal (f.readchar()) ""
		assert_string_equal (f.peekchar()) ""
		assert_string_equal (f.readchar()) ""
		assert_string_equal (f.peekchar()) ""
		assert_string_equal (f.readchar()) ""
		assert_string_equal (f.peekchar()) ""
		assert_string_equal (f.readchar()) ""
		assert_string_equal (f.peekchar()) ""
		assert_string_equal (f.readchar()) ""
		memStreamMgr.close f

		s = "H\r"
		f = memStreamMgr.openString s
		assert_false (f.eos())
		assert_string_equal (f.peekchar()) "H"
		assert_string_equal (f.readchar()) "H"
		assert_false (f.eos())
		assert_string_equal (f.peekchar()) "\r"
		assert_string_equal (f.readchar()) "\r"
		assert_true (f.eos())
		memStreamMgr.close f
	),
	
	fn test_getOSVersion = 
	(
		print "running test_getOSVersion"
		local osVersion = systemTools.GetOSVersion()
		assert_equal 3 osVersion.count
		assert_not_equal 0 osVersion[1] -- major version
--		assert_not_equal 0 osVersion[2]  -- minor version may be == 0
		assert_not_equal 0 osVersion[3] -- build number
	),
	
	fn test_maxversion =
	(
		print "running test_maxversion"
		version_info = maxversion() -- #(21000, 52, 0, 21, 2, 0, 0, 2019, ".2 Update")
		buildNumbers = filterstring (systemTools.GetBuildNumber()) "." -- #("21", "2", "0", "0")
		assert_equal 9 version_info.count
		assert_equal 4 buildNumbers.count
		assert_equal (buildNumbers[1] as integer) (version_info[4])
		assert_equal (buildNumbers[2] as integer) (version_info[5])
		assert_equal (buildNumbers[3] as integer) (version_info[6])
		assert_equal (buildNumbers[4] as integer) (version_info[7])
		assert_not_equal 0 (version_info[4])
	),
	
	fn test_getCommandLine =
	(
		print "running test_getCommandLine"
		deletefile @"$temp\test_getCommandLine.txt"
		local thisFileName = getSourceFileName()
		local thisDir = pathConfig.removePathLeaf thisFileName
		local scriptFile =  thisDir + "\\test_getCommandLine.mxs"
		local maxio_exe_path = symbolicPaths.getPathValue"$max" + "\\3dsmaxio.exe"
		local max_exe = "3dsmax.exe"
		if doesFileExist maxio_exe_path then
		(
			max_exe = "3dsmaxio.exe"
		)
        local cmd_log = pathConfig.appendPath (getDir #temp) "mxsagni_misc_Tests_hiddenDOScmd_output.log"
		local commandline = "\"\"" + symbolicPaths.getPathValue"$max" + "\\" + max_exe + "\" -silent -U MAXScript \"" + scriptFile + "\"" + " > \"" + cmd_log + "\"\""
		format "test_getCommandLine - running command: %\n" commandline
		HiddenDosCommand commandline donotwait:false ExitCode:&exitcode3dsmax
		assert_equal 0 exitcode3dsmax message:(" 3ds Max exit code is not 0: " + (exitcode3dsmax as string) )

        if (exitcode3dsmax != 0) do (
            fin = openfile cmd_log mode:"r"
            seek fin #eof
            maxlen = filepos fin
            seek fin 0
            res = readChars fin maxlen errorAtEOF:false
            format "3dsmax output:\n"
            format "%\n" res
            close fin
        )

		local f = openfile @"$temp\test_getCommandLine.txt"
		local l0 = readline f
		local l1 = readline f
		local l2 = readValue f
		close f

		/* This test for "save required" flags is unrelated to getCommandLine. It would be better suited to FileIO.UnitTest.ms.
		The goal is to validate the values of these flags BEFORE any scene reset is performed. This is only possible in tests
		that use a secondary 3ds Max instance (like here). In normal tests, the framework always resets the scene before running tests.
		I didn't want to add a secondary instance in FileIO.UnitTest.ms to avoid increasing test duration too much.
		I preferred to add this to an existing test instead	*/
		assert_equal "false,false" l0 message:"Unexpected values for getSaveRequired() and/or autosave.isRequired() right after startup"

		assert_defined (findstring l1 max_exe)
		assert_defined (findstring l1 "-silent")
		assert_defined (findstring l1 "-U")
		assert_defined (findstring l1 "MAXScript")
		assert_defined (findstring l1 "test_getCommandLine")
		assert_equal "-silent" (l2[1])
		assert_equal "-U" (l2[2])
		assert_equal "MAXScript" (l2[3])
		assert_equal scriptFile (l2[4])
		assert_equal 4 l2.count
	),
	
	fn test_sysinfo_getMAX_XXX_Count =
	(
		print "running test_sysinfo_getMAX_XXX_Count"
		local h = sysinfo.getMAXHandleCount()
		assert_not_equal 0 h
		assert_equal Integer (classof h)
		local u = sysinfo.getMAXUserObjectCount()
		assert_not_equal 0 u
		assert_equal Integer (classof u)
		local g = sysinfo.getMAXGDIObjectCount()
		assert_not_equal 0 g
		assert_equal Integer (classof g)
	),
  
	fn test_min_max =
	(
		print "running test_min_max"
		local vals = #(4,1,2,3)
		local vals_copy = copy vals #nomap
		local max_val = amax vals
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		local min_val = amin vals
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(1,2,3,4)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin vals
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(1,2,4,3)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin vals
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(1,2,0.5d0,3,4.4, 4)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 4.4 max_val
		assert_equal Float (classof max_val)
		min_val = amin vals
		assert_equal 0.5d0 min_val
		assert_equal Double (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(0.5d0,1,2,3,4,4.4)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 4.4 max_val
		assert_equal Float (classof max_val)
		min_val = amin vals
		assert_equal 0.5d0 min_val
		assert_equal Double (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(0.5d0,1,0,5,2,3,4,4.4)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 5 max_val
		assert_equal Integer (classof max_val)
		min_val = amin vals
		assert_equal 0 min_val
		assert_equal Integer (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(2L,1,4,3P)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin vals
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"

		vals = #(2L,1L,4P,3P)
		vals_copy = copy vals #nomap
		max_val = amax vals
		assert_equal 4P max_val
		assert_equal IntegerPtr (classof max_val)
		min_val = amin vals
		assert_equal 1L min_val
		assert_equal Integer64 (classof min_val)
		assert_array_equal vals vals_copy message:"input array modified"
		
		max_val = amax 4 1 2 3
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin 4 1 2 3
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)

		max_val = amax 1 2 3 4
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin 1 2 3 4
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)

		max_val = amax 1 2 4 3
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin 1 2 4 3
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)

		max_val = amax 1 2 0.5d0 3 4.4  4
		assert_equal 4.4 max_val
		assert_equal Float (classof max_val)
		min_val = amin 1 2 0.5d0 3 4.4  4
		assert_equal 0.5d0 min_val
		assert_equal Double (classof min_val)

		max_val = amax 0.5d0 1 2 3 4 4.4
		assert_equal 4.4 max_val
		assert_equal Float (classof max_val)
		min_val = amin 0.5d0 1 2 3 4 4.4
		assert_equal 0.5d0 min_val
		assert_equal Double (classof min_val)

		max_val = amax 0.5d0 1 0 5 2 3 4 4.4
		assert_equal 5 max_val
		assert_equal Integer (classof max_val)
		min_val = amin 0.5d0 1 0 5 2 3 4 4.4
		assert_equal 0 min_val
		assert_equal Integer (classof min_val)

		max_val = amax 2L 1 4 3P
		assert_equal 4 max_val
		assert_equal Integer (classof max_val)
		min_val = amin 2L 1 4 3P
		assert_equal 1 min_val
		assert_equal Integer (classof min_val)

		max_val = amax 2L 1L 4P 3P
		assert_equal 4P max_val
		assert_equal IntegerPtr (classof max_val)
		min_val = amin 2L 1L 4P 3P
		assert_equal 1L min_val
		assert_equal Integer64 (classof min_val)
		
		max_val = amax #(1.9, 2)
		assert_equal 2 max_val
		assert_equal Integer (classof max_val)
		max_val = amax #(2.1, 2)
		assert_equal 2.1 max_val
		assert_equal Float (classof max_val)
		max_val = amax #(2, 1.9)
		assert_equal 2 max_val
		assert_equal Integer (classof max_val)
		max_val = amax #(2, 2.1)
		assert_equal 2.1 max_val
		assert_equal Float (classof max_val)

		max_val = amax #(-1.9, -2)
		assert_equal -1.9 max_val
		assert_equal Float (classof max_val)
		max_val = amax #(-2.1, -2)
		assert_equal -2 max_val
		assert_equal Integer (classof max_val)
		max_val = amax #(-2, -1.9)
		assert_equal -1.9 max_val
		assert_equal Float (classof max_val)
		max_val = amax #(-2, -2.1)
		assert_equal -2 max_val
		assert_equal Integer (classof max_val)

		max_val = amax 1.9  2
		assert_equal 2 max_val
		assert_equal Integer (classof max_val)
		max_val = amax 2.1  2
		assert_equal 2.1 max_val
		assert_equal Float (classof max_val)
		max_val = amax 2  1.9
		assert_equal 2 max_val
		assert_equal Integer (classof max_val)
		max_val = amax 2  2.1
		assert_equal 2.1 max_val
		assert_equal Float (classof max_val)

		max_val = amax -1.9  -2
		assert_equal -1.9 max_val
		assert_equal Float (classof max_val)
		max_val = amax -2.1  -2
		assert_equal -2 max_val
		assert_equal Integer (classof max_val)
		max_val = amax -2  -1.9
		assert_equal -1.9 max_val
		assert_equal Float (classof max_val)
		max_val = amax -2  -2.1
		assert_equal -2 max_val
		assert_equal Integer (classof max_val)

		min_val = amin #(1.9, 2)
		assert_equal 1.9 min_val
		assert_equal Float (classof min_val)
		min_val = amin #(2.1, 2)
		assert_equal 2 min_val
		assert_equal Integer (classof min_val)
		min_val = amin #(2, 1.9)
		assert_equal 1.9 min_val
		assert_equal Float (classof min_val)
		min_val = amin #(2, 2.1)
		assert_equal 2 min_val
		assert_equal Integer (classof min_val)

		min_val = amin #(-1.9, -2)
		assert_equal -2 min_val
		assert_equal Integer (classof min_val)
		min_val = amin #(-2.1, -2)
		assert_equal -2.1 min_val
		assert_equal Float (classof min_val)
		min_val = amin #(-2, -1.9)
		assert_equal -2 min_val
		assert_equal Integer (classof min_val)
		min_val = amin #(-2, -2.1)
		assert_equal -2.1 min_val
		assert_equal Float (classof min_val)

		min_val = amin 1.9  2
		assert_equal 1.9 min_val
		assert_equal Float (classof min_val)
		min_val = amin 2.1  2
		assert_equal 2 min_val
		assert_equal Integer (classof min_val)
		min_val = amin 2  1.9
		assert_equal 1.9 min_val
		assert_equal Float (classof min_val)
		min_val = amin 2  2.1
		assert_equal 2 min_val
		assert_equal Integer (classof min_val)

		min_val = amin -1.9  -2
		assert_equal -2 min_val
		assert_equal Integer (classof min_val)
		min_val = amin -2.1  -2
		assert_equal -2.1 min_val
		assert_equal Float (classof min_val)
		min_val = amin -2  -1.9
		assert_equal -2 min_val
		assert_equal Integer (classof min_val)
		min_val = amin -2  -2.1
		assert_equal -2.1 min_val
		assert_equal Float (classof min_val)
	),

	fn test_formattedPrint_invalid_arg =
	(
		print "running test_formattedPrint_invalid_arg"
		-- test to make sure no crash and that exception is thrown
		local exception_occurred = false
		try
		(
			formattedprint pi format:"2,3f" -- format specification invalid
		)
		catch
		(
			exception_occurred = true
		)
		assert_true exception_occurred
		exception_occurred = false
		local res 
		try
		(
			res = formattedprint pi format:"2.3f" -- format specification valid
		)
		catch
		(
			exception_occurred = true
		)
		assert_false exception_occurred
		assert_string_equal "3.142" res
	),

	fn test_ParamType_To_from_TypeName =
	(
		print "running test_ParamType_To_from_TypeName"
		local type = typeNameToParamType #float
		assert_equal 0 type message:"incorrect type for #float"
		local typeName = paramTypeToTypeName 0
		assert_equal #float typeName message:"incorrect typeName for 0"
		type = typeNameToParamType #integer
		assert_equal 1 type message:"incorrect type for #integer"
		typeName = paramTypeToTypeName 1
		assert_equal #integer typeName message:"incorrect typeName for 1"
		type = typeNameToParamType #point2Tab
		assert_equal 2078 type message:"incorrect type for #point2Tab"
		typeName = paramTypeToTypeName 2078
		assert_equal #point2Tab typeName message:"incorrect typeName for 2078"
		type = typeNameToParamType #xxxxxxx
		assert_equal undefined type message:"type returned for undefined typeName"
		typeName = paramTypeToTypeName 9999999
		assert_equal undefined typeName message:"typeName returned for undefined type"
	),
	
	fn test_fileProperties_addProperty_date =
	(
		print "running test_fileProperties_addProperty_date"
		theDate = (filterstring localtime " ")[1]
		fileProperties.addProperty #custom "DateVal" theDate #date
		index = fileProperties.findProperty #custom "DateVal" 
		assert_not_equal 0 index message:"property not found"
		returnedDate = fileproperties.getPropertyValue #custom index
		assert_string_equal theDate returnedDate
	),
	
	fn test_BigMatrixValue_BigMatrixRowArray  =
	(
		print "running test_BigMatrixValue_BigMatrixRowArray"
		q = bigMatrix 3 4
		assert_equal 3 q.rows
		assert_equal 4 q.columns
		n = 0
		for i = 1 to 3 do
			for j = 1 to 4 do
				q[i][j] = (n += 1)
		row1 = q[1]
		row3 = q[3]
		row3_copy = row3 as array
		q[1] = #(-1.,-2.,-3.,-4.)
		assert_array_equal #(-1.,-2.,-3.,-4.) (q[1] as array)
		assert_array_equal #(-1.,-2.,-3.,-4.) (row1 as array)
		assert_array_equal row3_copy (row3 as array)
		q[1][4] = 4
		assert_array_equal #(-1.,-2.,-3.,4.) (row1 as array)
		assert_array_equal row3_copy (row3 as array)

		-- make smaller
		setSize q 2 3
		assert_equal 2 q.rows
		assert_equal 3 q.columns
		assert_array_equal #(0.,0.,0.) (q[1] as array)
		assert_array_equal #(0.,0.,0.) (row1 as array)
		exception_occurred = false
		try ( row3[1] )
		catch ( exception_occurred = true )
		assert_true exception_occurred
		exception_occurred = false
		try ( row3[1] = 1 )
		catch ( exception_occurred = true )
		assert_true exception_occurred
		exception_occurred = false
		try ( row3 as array )
		catch ( exception_occurred = true )
		assert_true exception_occurred
		try ( row1[4] = 1 )
		catch ( exception_occurred = true )
		assert_true exception_occurred

		-- make bigger
		setSize q 5 6
		assert_equal 5 q.rows
		assert_equal 6 q.columns
		assert_array_equal #(0.,0.,0.,0.,0.,0.) (q[1] as array)
		assert_array_equal #(0.,0.,0.,0.,0.,0.) (row1 as array)
		assert_array_equal #(0.,0.,0.,0.,0.,0.) (q[5] as array)
		q[5][6] = 1.
		assert_equal 1. (q[5][6] )
		assert_array_equal #(0.,0.,0.,0.,0.,1.) (q[5] as array)

		-- clear
		clear q
		assert_equal 0 q.rows
		assert_equal 0 q.columns
		exception_occurred = false
		try ( row1[1] = 1 )
		catch ( exception_occurred = true )
		assert_true exception_occurred
	),
	
	fn test_refs_dependnodes =
	(
		print "running test_refs_dependnodes"
		local a = sphere() 
		local res = refs.dependentNodes a.baseobject firstOnly:true
		assert_equal a res message:"node not found - a.baseobject firstOnly:true"
		res = refs.dependentNodes a.baseobject 
		assert_equal 1 res.count message:"node not found - a.baseobject"
		assert_equal a res[1] message:"incorrect node found - a.baseobject"
		res = refs.dependentNodes a.baseobject firstOnly:true baseObjectOnly:true
		assert_equal a res message:"node not found - a.baseobject firstOnly:true baseObjectOnly:true"
		res = refs.dependentNodes a.baseobject baseObjectOnly:true
		assert_equal 1 res.count message:"node not found - a.baseobject baseObjectOnly:true"
		assert_equal a res[1] message:"incorrect node found - a.baseobject baseObjectOnly:true"

		res = refs.dependentNodes a.transform.controller firstOnly:true
		assert_equal a res message:"node not found - a.transform.controller firstOnly:true"
		res = refs.dependentNodes a.transform.controller 
		assert_equal 1 res.count message:"node not found - a.transform.controller"
		assert_equal a res[1] message:"incorrect node found - a.transform.controller"
		res = refs.dependentNodes a.transform.controller firstOnly:true baseObjectOnly:true
		assert_undefined res message:"node found when shouldn't - a.transform.controller firstOnly:true baseObjectOnly:true"
		res = refs.dependentNodes a.transform.controller baseObjectOnly:true
		assert_equal 0 res.count message:"node found when shouldn't - a.transform.controller baseObjectOnly:true"
		
		local b = bend()
		addmodifier a b 
		res = refs.dependentNodes b firstOnly:true
		assert_equal a res message:"node not found - b firstOnly:true"
		res = refs.dependentNodes b
		assert_equal 1 res.count message:"node not found - b"
		assert_equal a res[1] message:"incorrect node found - b"
		res = refs.dependentNodes b firstOnly:true baseObjectOnly:true
		assert_undefined res message:"node found when shouldn't - b firstOnly:true baseObjectOnly:true"
		res = refs.dependentNodes b baseObjectOnly:true
		assert_equal 0 res.count message:"node found when shouldn't - b baseObjectOnly:true"
		
		local aa=reference a --> $Editable_Mesh:Sphere002 @ [0.000000,0.000000,0.000000]
		local t = twist()
		addmodifier aa t
		
		res = refs.dependentNodes a.baseobject firstOnly:true
		assert_equal aa res message:"incorrect or no node found - a.baseobject firstOnly:true"
		res = refs.dependentNodes a.baseobject
		assert_equal 2 res.count message:"all nodes not found - a.baseobject"
		assert_equal aa res[1] message:"incorrect node 1 found - a.baseobject"
		assert_equal a res[2] message:"incorrect node 2 found - a.baseobject"
		res = refs.dependentNodes a.baseobject firstOnly:true baseObjectOnly:true
		assert_equal aa res message:"incorrect or no node found - a.baseobject firstOnly:true baseObjectOnly:true"
		res = refs.dependentNodes a.baseobject baseObjectOnly:true
		assert_equal 2 res.count message:"all nodes not found - a.baseobject baseObjectOnly:true"
		assert_equal aa res[1] message:"incorrect node 1 found - a.baseobject baseObjectOnly:true"
		assert_equal a res[2] message:"incorrect node 2 found - a.baseobject baseObjectOnly:true"
		
		res = refs.dependentNodes b firstOnly:true
		assert_equal aa res message:"incorrect or no node found - b firstOnly:true"
		res = refs.dependentNodes b
		assert_equal 2 res.count message:"all nodes not found - b"
		assert_equal aa res[1] message:"incorrect node 1 found - b"
		assert_equal a res[2] message:"incorrect node 2 found - b"
		res = refs.dependentNodes b firstOnly:true baseObjectOnly:true
		assert_undefined res message:"node found when shouldn't - b firstOnly:true baseObjectOnly:true"
		res = refs.dependentNodes b baseObjectOnly:true
		assert_equal 0 res.count message:"node found when shouldn't - b baseObjectOnly:true"
		
		res = refs.dependentNodes t firstOnly:true
		assert_equal aa res message:"incorrect or no node found - t firstOnly:true"
		res = refs.dependentNodes t	
		assert_equal 1 res.count message:"incorrect number of nodes found - t"
		assert_equal aa res[1] message:"incorrect node found - t"
		res = refs.dependentNodes t firstOnly:true baseObjectOnly:true
		assert_undefined res message:"node found when shouldn't - t firstOnly:true baseObjectOnly:true"
		res = refs.dependentNodes t baseObjectOnly:true
		assert_equal 0 res.count message:"node found when shouldn't - t baseObjectOnly:true"
	),
	
	fn test_fileProperties_addProperty_before_after_save =
	(
		print "running test_fileProperties_addProperty_before_after_save"
		resetmaxfile #noprompt
		n = fileProperties.getNumProperties #contents
		assert_equal 0 n message:"unexpected #contents properties found"
		if n != 0 do
			for i = 1 to n do format "% : % :%\n" i (fileProperties.getPropertyName #contents i) (fileProperties.getPropertyValue #contents i)
		fileProperties.addProperty #contents "Manager" "xx"
		n = fileProperties.getNumProperties #contents
		assert_equal 1 n message:"incorrect #contents properties count after addProperty"
		propName = fileProperties.getPropertyName #contents 1
		assert_string_equal "Manager" propName message:"incorrect prop name"
		propVal = fileProperties.getPropertyValue #contents 1
		assert_string_equal "xx" propVal message:"incorrect prop val"
		if n != 1 do
			for i = 1 to n do format "% : % :%\n" i (fileProperties.getPropertyName #contents i) (fileProperties.getPropertyValue #contents i)
		box()
		savemaxfile @"$temp\test_fileProperties.max" quiet:true
		loadmaxfile @"$temp\test_fileProperties.max" quiet:true
		n = fileProperties.getNumProperties #contents
		assert_equal 2 n message:"incorrect #contents properties count after loadmaxfile"
		propName = fileProperties.getPropertyName #contents 1
		assert_string_equal "Manager" propName message:"incorrect prop name"
		propVal = fileProperties.getPropertyValue #contents 1
		assert_string_equal "xx" propVal message:"incorrect prop val"
		propName = fileProperties.getPropertyName #contents 2
		assert_string_equal "Headers" propName message:"incorrect prop name"
		propVal = fileProperties.getPropertyValue #contents 2
		assert_array_equal #("General", "Mesh Totals", "Scene Totals", "External Dependencies", "Objects", "Materials", "Used Plug-Ins", "Render Data") propVal message:"incorrect prop val"
		if n != 2 do
			for i = 1 to n do format "% : % :%\n" i (fileProperties.getPropertyName #contents i) (fileProperties.getPropertyValue #contents i)
		fileProperties.addProperty #contents "Company" "yy"
		n = fileProperties.getNumProperties #contents
		assert_equal 3 n message:"incorrect #contents properties count after addProperty"
		propName = fileProperties.getPropertyName #contents 3
		assert_string_equal "Company" propName message:"incorrect prop name"
		propVal = fileProperties.getPropertyValue #contents 3
		assert_string_equal "yy" propVal message:"incorrect prop val"
		if n != 3 do
			for i = 1 to n do format "% : % :%\n" i (fileProperties.getPropertyName #contents i) (fileProperties.getPropertyValue #contents i)
		savemaxfile @"$temp\test_fileProperties.max" quiet:true
		loadmaxfile @"$temp\test_fileProperties.max" quiet:true
		n = fileProperties.getNumProperties #contents
		assert_equal 3 n message:"incorrect #contents properties count after loadmaxfile"
		propName = fileProperties.getPropertyName #contents 2
		assert_string_equal "Company" propName message:"incorrect prop name"
		propName = fileProperties.getPropertyName #contents 3 -- Headers was removed/added during save/load
		assert_string_equal "Headers" propName message:"incorrect prop name"
	),
	
	fn test_getclassinstances_inner =
	(
		-- getClassInstances <MAXClass> target:<MAXObject> asTrackViewPick:<bool> processChildren:<bool> processAllAnimatables:<bool>
		-- getClassInstances #(<MAXClass>, ...) target:<MAXObject> asTrackViewPick:<bool> processChildren:<bool> processAllAnimatables:<bool>
		-- getClassInstances #all target:<MAXObject> asTrackViewPick:<bool> processChildren:<bool> processAllAnimatables:<bool>
		
		resetmaxfile #noprompt
		gc()
		
		local numExistingBendMods = (getClassInstances bend processAllAnimatables:true).count
		local numExistingBoxes = (getClassInstances box processAllAnimatables:true).count
		local numExistingNodeObjects = (getClassInstances nodeobject processAllAnimatables:true).count
		-- format "numExistingBendMods:% numExistingBoxes:% numExistingNodeObjects:%\n" numExistingBendMods numExistingBoxes numExistingNodeObjects
		
		local box1=box()
		local bend1= bend()
		local bend2 = bend()
		addmodifier box1 bend1
		addmodifier box1 bend2
		local sphere1 = sphere parent:box1
		local bend3 = bend()
		addmodifier sphere1 bend3
		local bend4 = bend()
		
		local res1 = getClassInstances bend
		assert_equal 3 res1.count 
		assert_not_equal 0 (finditem res1 bend1)
		assert_not_equal 0 (finditem res1 bend2)
		assert_not_equal 0 (finditem res1 bend3)
		
		local res2a = getClassInstances bend target:box1
		assert_equal 2 res2a.count 
		assert_not_equal 0 (finditem res2a bend1)
		assert_not_equal 0 (finditem res2a bend2)
		local res2b = getClassInstances bend target:box1 processChildren:true
		assert_equal 3 res2b.count 
		assert_not_equal 0 (finditem res2b bend1)
		assert_not_equal 0 (finditem res2b bend2)
		assert_not_equal 0 (finditem res2b bend3)
		
		local res3a = getClassInstances bend target:box1 asTrackViewPick:true
		assert_equal 2 res3a.count 
		local res3a = for r in res3a collect r.anim
		assert_not_equal 0 (finditem res3a bend1)
		assert_not_equal 0 (finditem res3a bend2)
		local res3b = getClassInstances bend target:box1 processChildren:true asTrackViewPick:true
		assert_equal 3 res3b.count 
		local res3b = for r in res3b collect r.anim
		assert_not_equal 0 (finditem res3b bend1)
		assert_not_equal 0 (finditem res3b bend2)
		assert_not_equal 0 (finditem res3b bend3)
		
		local res4 = getClassInstances bend processAllAnimatables:true
		assert_equal 4 (res4.count - numExistingBendMods)
		assert_not_equal 0 (finditem res4 bend1)
		assert_not_equal 0 (finditem res4 bend2)
		assert_not_equal 0 (finditem res4 bend3)
		assert_not_equal 0 (finditem res4 bend4)

		
		local res5 = getClassInstances #(bend,box)
		assert_equal Dictionary (classof res5)
		assert_equal 2 res5.count 
		assert_equal 3 res5[bend].count 
		assert_not_equal 0 (finditem res5[bend] bend1)
		assert_not_equal 0 (finditem res5[bend] bend2)
		assert_not_equal 0 (finditem res5[bend] bend3)
		assert_equal 1 res5[box].count
		assert_not_equal 0 (finditem res5[box] box1.baseobject)
		
		local res6a = getClassInstances #(bend,box) target:box1
		assert_equal 2 res6a.count 
		assert_equal 2 res6a[bend].count 
		assert_not_equal 0 (finditem res6a[bend] bend1)
		assert_not_equal 0 (finditem res6a[bend] bend2)
		assert_equal 1 res6a[box].count 
		assert_not_equal 0 (finditem res6a[box] box1.baseobject)
		local res6b = getClassInstances #(bend,box) target:box1 processChildren:true
		assert_equal 2 res6b.count 
		assert_equal 3 res6b[bend].count 
		assert_not_equal 0 (finditem res6b[bend] bend1)
		assert_not_equal 0 (finditem res6b[bend] bend2)
		assert_not_equal 0 (finditem res6b[bend] bend3)
		assert_equal 1 res6b[box].count 
		assert_not_equal 0 (finditem res6b[box] box1.baseobject)
		
		local res7a = getClassInstances #(bend,box) target:box1 asTrackViewPick:true
		assert_equal 2 res7a.count 
		assert_equal 2 res7a[bend].count 
		local res7a_bend = for r in res7a[bend] collect r.anim
		assert_not_equal 0 (finditem res7a_bend bend1)
		assert_not_equal 0 (finditem res7a_bend bend2)
		assert_equal 1 res7a[box].count 
		local res7a_box = for r in res7a[box] collect r.anim
		assert_not_equal 0 (finditem res7a_box box1.baseobject)
		local res7b = getClassInstances #(bend,box) target:box1 processChildren:true asTrackViewPick:true
		assert_equal 2 res7b.count 
		assert_equal 3 res7b[bend].count 
		local res7b_bend = for r in res7b[bend] collect r.anim
		assert_not_equal 0 (finditem res7b_bend bend1)
		assert_not_equal 0 (finditem res7b_bend bend2)
		assert_not_equal 0 (finditem res7b_bend bend3)
		assert_equal 1 res7b[box].count 
		local res7b_box = for r in res7b[box] collect r.anim
		assert_not_equal 0 (finditem res7b_box box1.baseobject)
		
		local res8 = getClassInstances #(bend,box) processAllAnimatables:true
		assert_equal 2 res8.count 
		assert_equal 4 (res8[bend].count - numExistingBendMods) 
		assert_not_equal 0 (finditem res8[bend] bend1)
		assert_not_equal 0 (finditem res8[bend] bend2)
		assert_not_equal 0 (finditem res8[bend] bend3)
		assert_not_equal 0 (finditem res8[bend] bend4)
		assert_equal 1 (res8[box].count - numExistingBoxes)
		assert_not_equal 0 (finditem res8[box] box1.baseobject)

		
		local res9 = getClassInstances #all
		assert_equal Dictionary (classof res9)
		assert_equal 3 res9[bend].count 
		assert_not_equal 0 (finditem res9[bend] bend1)
		assert_not_equal 0 (finditem res9[bend] bend2)
		assert_not_equal 0 (finditem res9[bend] bend3)
		assert_equal 1 res9[box].count
		assert_not_equal 0 (finditem res9[box] box1.baseobject)
		assert_equal 2 res9[nodeobject].count
		assert_not_equal 0 (finditem res9[nodeobject] box1)
		assert_not_equal 0 (finditem res9[nodeobject] sphere1)
		
		local res10a = getClassInstances #all target:box1
		assert_equal 2 res10a[bend].count 
		assert_not_equal 0 (finditem res10a[bend] bend1)
		assert_not_equal 0 (finditem res10a[bend] bend2)
		assert_equal 1 res10a[box].count 
		assert_not_equal 0 (finditem res10a[box] box1.baseobject)
		assert_equal 1 res10a[nodeobject].count 
		assert_not_equal 0 (finditem res10a[nodeobject] box1)
		local res10b = getClassInstances #all target:box1 processChildren:true
		assert_equal 3 res10b[bend].count 
		assert_not_equal 0 (finditem res10b[bend] bend1)
		assert_not_equal 0 (finditem res10b[bend] bend2)
		assert_not_equal 0 (finditem res10b[bend] bend3)
		assert_equal 1 res10b[box].count 
		assert_not_equal 0 (finditem res10b[box] box1.baseobject)
		assert_equal 2 res10b[nodeobject].count 
		assert_not_equal 0 (finditem res10b[nodeobject] box1)
		assert_not_equal 0 (finditem res10b[nodeobject] sphere1)
		
		local res11a = getClassInstances #all target:box1 asTrackViewPick:true
		assert_equal 2 res11a[bend].count 
		local res11a_bend = for r in res11a[bend] collect r.anim
		assert_not_equal 0 (finditem res11a_bend bend1)
		assert_not_equal 0 (finditem res11a_bend bend2)
		assert_equal 1 res11a[box].count 
		local res11a_box = for r in res11a[box] collect r.anim
		assert_not_equal 0 (finditem res11a_box box1.baseobject)
		assert_equal 1 res11a[nodeobject].count 
		local res11a_nodeobject = for r in res11a[nodeobject] collect r.anim
		assert_not_equal 0 (finditem res11a_nodeobject box1)
		local res11b = getClassInstances #all target:box1 processChildren:true asTrackViewPick:true
		assert_equal 3 res11b[bend].count 
		local res11b_bend = for r in res11b[bend] collect r.anim
		assert_not_equal 0 (finditem res11b_bend bend1)
		assert_not_equal 0 (finditem res11b_bend bend2)
		assert_not_equal 0 (finditem res11b_bend bend3)
		assert_equal 1 res11b[box].count 
		local res11b_box = for r in res11b[box] collect r.anim
		assert_not_equal 0 (finditem res11b_box box1.baseobject)
		assert_equal 2 res11b[nodeobject].count 
		local res11b_nodeobject = for r in res11b[nodeobject] collect r.anim
		assert_not_equal 0 (finditem res11b_nodeobject box1)
		assert_not_equal 0 (finditem res11b_nodeobject sphere1)
		
		local res12 = getClassInstances #all processAllAnimatables:true
		assert_equal 4 (res12[bend].count - numExistingBendMods)
		assert_not_equal 0 (finditem res12[bend] bend1)
		assert_not_equal 0 (finditem res12[bend] bend2)
		assert_not_equal 0 (finditem res12[bend] bend3)
		assert_not_equal 0 (finditem res12[bend] bend4)
		assert_equal 1 (res12[box].count - numExistingBoxes)
		assert_not_equal 0 (finditem res12[box] box1.baseobject)
		assert_equal 2 (res12[nodeobject].count - numExistingNodeObjects)
		assert_not_equal 0 (finditem res12[nodeobject] box1)
		assert_not_equal 0 (finditem res12[nodeobject] sphere1)
	),
	
	fn test_getclassinstances =
	(
		print "running test_getclassinstances"
		gc()
		test_getclassinstances_inner()
		gc()
		resetMaxFile quiet:true
	),
	
	fn assert_dict_equal d1 d2 which =
	(
		format "testing %\n" which
		assert_equal d1.count d2.count message:("dictionary size mismatch for "+ which)
		areEqual = deepequal d1 d2
		assert_true areEqual message:("sceneProperties != sceneFileProperties "+ which)
		if not areEqual do
		(
			for ele in d1 do
			(
				found = HasDictValue d2 ele.key
				assert_true found message:("content mismatch in " + which + " item " + ele.key)
				if found do
				(
					d1Val = ele.value
					d2Val = d2[ele.key]
					if isKindOf d1Val Array and isKindOf d2Val Array then
						assert_array_equal d1Val d2Val message:("content value mismatch in " + which + ":" + ele.key)
					else if isKindOf d1Val Dictionary and isKindOf d2Val Dictionary then
						assert_dict_equal d1Val d2Val (which + ":" + ele.key)
					else
						assert_equal d1Val d2Val message:("content value mismatch in " + which + ":" + ele.key)
				)
			)
		)
	),

	fn getMAXFileFileProperties_scenePrep fname =
	(
		resetmaxfile #noprompt
		teapot()
		circle()
		Freecamera()
		Omnilight()
		dummy()

		-- Add some properties
		fileProperties.addProperty #summary "Title" "H2G2"
		fileProperties.addProperty #summary "Subject" "Earth" 
		fileProperties.addProperty #summary "Author" "Ford Prefect"

		fileProperties.addProperty #contents "Manager" "Zarniwoop" 
		fileProperties.addProperty #contents "Company" "Megadodo Publishing" 
		fileProperties.addProperty #contents "Category" "Mostly Harmless" 

		fileProperties.addProperty #custom "IntVal" 42
		fileProperties.addProperty #custom "FloatVal" 1.234
		fileProperties.addProperty #custom "BoolVal" true
		fileProperties.addProperty #custom "DateVal" "05/11/2001" #date
		--
		saveMaxFile fname quiet:true
	),

	-- resave:true specifies that if scene and file properties are not equal, whether to save/load the scene file and 
	-- reacquire the scene and file properties. Set this to true when loading potentially old files, as they may not
	-- contain some file properties such as Indentifier and Revision

	fn test_getMAXFileFileProperties_imp fname resave:false =
	(
		fn getSceneProperties =
		(
			local doPrint = false
			--
			-- Get all properties
			pages = #( #summary, #contents, #custom)
			dictictionaries = #(Dictionary #string, Dictionary #string, Dictionary #string)
			for pg, j in pages do
			(
				theDict = dictictionaries[j]
				if doPrint do
					format "--- % ---\n" (pg as string)
				for i=1 to (fileProperties.getNumProperties pg) do
				(
					local pname = (fileProperties.getPropertyName pg i)
					local pval = (fileProperties.getPropertyValue pg i)
					if doPrint do
						format "\t% : " pname
					if (pname =="Headers") then
					(
						headerDict = Dictionary #string
						if doPrint do
							format "\n"
						for hdr in pval do
						(
							if doPrint do
								format "\t\t%\n" hdr
							local docs = fileProperties.getItems hdr
							if docs != undefined then
							(
								items = #()
								for d in docs do 
								(
									if doPrint do
										format "\t\t\t%\n"d
									append items d
								)
								headerDict[hdr]=items
							)
						)
						theDict[pname]=headerDict
					)
					else 
					(
						if doPrint do
							format " %\n" pval
						theDict[pname]=pval
					)
				)
			)
			dictictionaries
		)

		-- load scene file
		local areEqual = true
		if (assert_true (loadMaxFile fname quiet:true) message:"loadMaxFile failed") do
		(
			local tfname = fname
			sceneProperties =  getSceneProperties()
			sceneFileProperties =  getMAXFileFileProperties tfname
			areEqual = deepequal sceneProperties sceneFileProperties
			-- if not equal, check whether should save/load/reacquire
			if not areEqual and resave do
			(
				format "resaving: %\n" fname
				tfname = "$scenes\getMAXFileFileProperties_temp.max"
				assert_true (saveMaxFile tfname quiet:true)
				assert_true (loadMaxFile tfname quiet:true)
				sceneProperties =  getSceneProperties()
				sceneFileProperties =  getMAXFileFileProperties tfname
			)
			
			areEqual = deepequal sceneProperties sceneFileProperties
			assert_true areEqual message:"sceneProperties != sceneFileProperties"
			if not areEqual do
			(
				pages = #( #summary, #contents, #custom)
				for pg, j in pages do
				(
					scenePropertiesDict = sceneProperties[j]
					sceneFilePropertiesDict = sceneFileProperties[j]
					assert_dict_equal scenePropertiesDict sceneFilePropertiesDict ("#" + pg as string)
				)
			)
		)
		areEqual
	),

	fn test_getMAXFileFileProperties =
	(
		fname = @"$scenes\getMAXFileFileProperties.max"
		getMAXFileFileProperties_scenePrep fname
		test_getMAXFileFileProperties_imp fname
	),	
	
	Tests = 
	#(
		test_sysinfo_MAXpriority_enum_values,
		test_get_set_ini,
		test_SingleRefMaker_TypedSingleRefMaker,
		test_used_in_scene,
		meshop_attach_test,
		meshop_polyop_get_listarg_test,
		meshop_getMatIdTests,
		polyop_getMatIdTests,
		test_getFaces,
		test_systemTools_sceneIODebugEnabled,
		test_getMAXClass,
		test_persistents,
		test_memStreamMgr_eos,
		test_getOSVersion,
		test_maxversion,
		test_getCommandLine,
		test_sysinfo_getMAX_XXX_Count,
		test_min_max,
		test_formattedPrint_invalid_arg,
		test_ParamType_To_from_TypeName,
		test_fileProperties_addProperty_date,
		test_BigMatrixValue_BigMatrixRowArray,
		test_refs_dependnodes,
		test_fileProperties_addProperty_before_after_save,
		test_getclassinstances,
		test_getMAXFileFileProperties
	)
)

run_test_fixture mxsagni_misc_Tests script:(getThisScriptFilename())
