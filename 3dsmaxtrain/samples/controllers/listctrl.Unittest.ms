fileIn "MxsUnit/MxsUnit.ms"

struct ListControl_TestFixture
(

	fn setUp =
	(
		resetMaxFile #noprompt 
	),
	
	fn tearDown =
	(
	),
	
	fn CompareResults tmListNode componentListNode =
	(
		-- Compare the final TM's from both nodes across the timeline
		for i = animationRange.start to animationRange.end do
		(
			at time i
			(
				tm = tmListNode.Transform
				comp = componentListNode.Transform
				
				tmRot = tm.rotationpart as eulerAngles
				tmTrans = tm.translationpart -- Point3
				tmScaleRot = tm.scalerotationpart as eulerAngles
				tmScale = tm.scalepart --Point3
				
				compRot = comp.rotationpart as eulerAngles
				compTrans = comp.translationpart -- Point3
				compScaleRot = comp.scalerotationpart as eulerAngles
				compScale = comp.scalepart --Point3
				
				assert_float_equal tmRot.x compRot.x  tolerance:0.001
				assert_float_equal tmRot.y compRot.y  tolerance:0.001
				assert_float_equal tmRot.z compRot.z  tolerance:0.001
				
				assert_point3_equal tmTrans compTrans tolerance:0.0001
				
				--assert_float_equal tmScaleRot.x compScaleRot.x  tolerance:0.001
				--assert_float_equal tmScaleRot.y compScaleRot.y  tolerance:0.001
				--assert_float_equal tmScaleRot.z compScaleRot.z  tolerance:0.001
				
				-- being more generous with the scale, seems to be less precise.
				assert_point3_equal tmScale compScale tolerance:0.005
				
			)
		)
	),
	
	fn SetupTransControllers tmListBox transListBox =
	(
		--Setup the position_list and transform_list on the node
		tmListBox.Transform.controller = transform_list()
		tmListBox.Transform.controller.available.controller = prs()
		
		transListBox.position.controller = Position_List()
		transListBox.position.controller.available.controller = Position_XYZ()
		
		--link the two position controllers together
		-- ex:
		-- tmListBox : tm_list
		--		-PRS
		--			-pos1 <- same
		--			-rot
		--			-scale
		--		-PRS
		--			-pos2 <- same
		--			-rot
		--			-scale
		-- transListBox : PRS
		--		-prs
		--			-position_list
		--				-pos1 <- same
		--				-pos2 <- same
		--			-rot
		--			-scale
		pos1 = transListBox.position.controller.getSubCtrl 1
		pos2 = transListBox.position.controller.getSubCtrl 2
		prs1 = tmListBox.transform.controller.getSubCtrl 1
		prs2 = tmListBox.transform.controller.getSubCtrl 2
		prs1.position.controller = pos1
		prs2.position.controller = pos2
		
		-- setup and share the same Weight
		with animate on
		(
			at time 0
			(
				tmListBox.transform.controller.weight = #(100.0, 0.0)
				transListBox.position.controller.weight = #(100.0, 0.0)
			)
			at time 100
			(
				tmListBox.transform.controller.weight = #(0.0, 100.0)
				transListBox.position.controller.weight = #(0.0, 100.0)
			)
		)
		
		
		-- Setup some animation on the pos controllers
		pos1.X_Position = -100
		pos1.Y_Position = 1
		pos1.Z_Position = 50
		pos2.X_Position = 100
		pos2.Y_Position = 0
		pos2.Z_Position = 0
		
		with animate on
		(
			at time 100
			(
				pos1.X_Position = 0
				pos1.Y_Position = 0
				pos1.Z_Position = 0
				pos2.X_Position = -1
				pos2.Y_Position = -20
				pos2.Z_Position = -300
			)
		)
	),

	fn tmListController_Trans =
	(
        -- Make a tm list control and a position list control and compare the results!
		format "tmListController_Trans\n"
		
		format "Setup\n"
		tmListBox = Box()
		tmListBox.name = "tmList"
		transListBox = Box()
		transListBox.name = "posList"
		
		--Setup the list controllers
		SetupTransControllers tmListBox transListBox
		
		-- Compare the final results
		format "Compare transform_list to position_list\n"
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		transListBox.position.controller.average = false
		CompareResults tmListBox transListBox
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		transListBox.position.controller.average = true
		CompareResults tmListBox transListBox
		delete tmListBox
		delete transListBox

		-- retest with a parent node
		format "Set a parent node and retest\n"
		tmListBox = Box()
		tmListBox.name = "tmList"
		transListBox = Box()
		transListBox.name = "posList"
		parentTM = sphere()
		parentTM.name = "parent"
		tmListBox.parent = parentTM
		transListBox.parent = parentTM
		-- Lastly setup a parent TM so the active tm list control doesn't get a rotation/translation setup on it.
		parentTM.position = [-10,-20,-30]
		parentTM.rotation = (eulerToQuat ((eulerAngles -45 10 -80)))
		-- Removed the scale on the parent because that introduces rotation in the TM list.
		--parentTM.scale = [0.5, 1, 2]
		
		--Setup the list controllers
		SetupTransControllers tmListBox transListBox
			
		-- Compare the final results again
		format "Compare again transform_list to position_list\n"
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		transListBox.position.controller.average = false
		CompareResults tmListBox transListBox

		-- KZ, Can only compare with sequential = false
		-- With sequential = true, rotations can be introduced and that will never look the same as pos_list.
		-- Ex: 40% of rotation X + 60% of rotation X != rotation X in this case.
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		transListBox.position.controller.average = true
		CompareResults tmListBox transListBox
		
		delete tmListBox
		delete transListBox
		delete parentTM
    ),
	
	fn SetupRotationControllers tmListBox rotListBox =
	(
		tmListBox.Transform.controller = transform_list()
		tmListBox.Transform.controller.available.controller = prs()
		
		rotListBox.rotation.controller = Euler_XYZ()
		rotListBox.rotation.controller = Rotation_List()
		rotListBox.rotation.controller.available.controller = Euler_XYZ()
		
		--link the two position controllers together
		-- ex:
		-- tmListBox : tm_list
		--		-PRS
		--			-pos
		--			-rot1 <- same
		--			-scale
		--		-PRS
		--			-pos
		--			-rot2 <- same
		--			-scale
		-- transListBox : PRS
		--		-prs
		--			-pos
		--			-rot_list
		--				-rot1 <- same
		--				-rot2 <- same
		--			-scale
		rot1 = rotListBox.rotation.controller.getSubCtrl 1
		rot2 = rotListBox.rotation.controller.getSubCtrl 2
		prs1 = tmListBox.transform.controller.getSubCtrl 1
		prs2 = tmListBox.transform.controller.getSubCtrl 2
		prs1.rotation.controller = rot1
		prs2.rotation.controller = rot2
		
		-- setup and share the same Weight
		with animate on
		(
			at time 0
			(
				tmListBox.transform.controller.weight = #(100.0, 0.0)
				rotListBox.rotation.controller.weight = #(100.0, 0.0)
			)
			at time 100
			(
				tmListBox.transform.controller.weight = #(0.0, 100.0)
				rotListBox.rotation.controller.weight = #(0.0, 100.0)
			)
		)
		
		
		-- Setup some animation
		rot1.X_Rotation = -100
		rot1.Y_Rotation = 1
		rot1.Z_Rotation = 50
		rot2.X_Rotation = 100
		rot2.Y_Rotation = 0
		rot2.Z_Rotation = 0
		
		with animate on
		(
			at time 100
			(
				rot1.X_Rotation = 0
				rot1.Y_Rotation = 0
				rot1.Z_Rotation = 0
				rot2.X_Rotation = -1
				rot2.Y_Rotation = -20
				rot2.Z_Rotation = -300
			)
		)
	),
	
	fn tmListController_Rotation = 
	(
		-- Make a tm list control and a rotation list control and compare the results!
		format "tmListController_Rotation\n"
		
		format "Setup\n"
		tmListBox = Box()
		tmListBox.name = "tmList"
		rotListBox = Box()
		rotListBox.name = "rotationList"
		SetupRotationControllers tmListBox rotListBox
		
		-- Compare the final results
		format "Compare transform_list to rotation_list\n"
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		rotListBox.rotation.controller.average = false
		CompareResults tmListBox rotListBox
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		rotListBox.rotation.controller.average = true
		CompareResults tmListBox rotListBox
		delete tmListBox
		delete rotListBox
		
		-- retest with a parent node
		format "Set a parent node and retest\n"
		tmListBox = Box()
		tmListBox.name = "tmList"
		rotListBox = Box()
		rotListBox.name = "rotationList"
		parentTM = sphere()
		parentTM.name = "parent"
		tmListBox.parent = parentTM
		rotListBox.parent = parentTM
		-- Lastly setup a parent TM so the active tm list control doesn't get a rotation/translation setup on it.
		parentTM.position = [-10,-20,-30]
		parentTM.rotation = (eulerToQuat ((eulerAngles -45 10 -80)))
		--parentTM.scale = [0.5, 1, 2]
		
		--Setup the list controllers
		SetupRotationControllers tmListBox rotListBox
		
		-- Compare the final results again
		format "Compare again transform_list to rotation_list\n"
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		rotListBox.rotation.controller.average = false
		CompareResults tmListBox rotListBox
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		rotListBox.rotation.controller.average = true
		CompareResults tmListBox rotListBox
		
		delete tmListBox
		delete rotListBox
		delete parentTM
	),
	
	fn SetupScaleControllers tmListBox scaleListBox =
	(
		tmListBox.Transform.controller = transform_list()
		tmListBox.Transform.controller.available.controller = prs()
		
		scaleListBox.scale.controller = ScaleXYZ()
		scaleListBox.scale.controller = Scale_List()
		scaleListBox.scale.controller.available.controller = ScaleXYZ()
		
		--link the two position controllers together
		-- ex:
		-- tmListBox : tm_list
		--		-PRS
		--			-pos
		--			-rot
		--			-scale1 <- same
		--		-PRS
		--			-pos
		--			-rot
		--			-scale2 <- same
		-- transListBox : PRS
		--		-prs
		--			-pos
		--			-rot
		--			-scale_list
		--				-scale1 <- same
		--				-scale2 <- same
		scale1 = scaleListBox.scale.controller.getSubCtrl 1
		scale2 = scaleListBox.scale.controller.getSubCtrl 2
		prs1 = tmListBox.transform.controller.getSubCtrl 1
		prs2 = tmListBox.transform.controller.getSubCtrl 2
		prs1.scale.controller = scale1
		prs2.scale.controller = scale2
		
		-- setup and share the same Weight
		with animate on
		(
			at time 0
			(
				tmListBox.transform.controller.weight = #(100.0, 0.0)
				scaleListBox.scale.controller.weight = #(100.0, 0.0)
			)
			at time 100
			(
				tmListBox.transform.controller.weight = #(0.0, 100.0)
				scaleListBox.scale.controller.weight = #(0.0, 100.0)
			)
		)
		
		
		-- Setup some animation
		scale1.X_Scale = 4
		scale1.Y_Scale = 1
		scale1.Z_Scale = 5
		scale2.X_Scale = 2
		scale2.Y_Scale = 0.1
		scale2.Z_Scale = 0.1
		
		with animate on
		(
			at time 100
			(
				scale1.X_Scale = 0.1
				scale1.Y_Scale = 0.1
				scale1.Z_Scale = 0.1
				scale2.X_Scale = 1
				scale2.Y_Scale = 2
				scale2.Z_Scale = 3
			)
		)
	),
	
	fn tmListController_Scale = 
	(
		-- Make a tm list control and a scale list control and compare the results!
		format "tmListController_Scale\n"

		tmListBox = Box()
		tmListBox.name = "tmList"
		scaleListBox = Box()
		scaleListBox.name = "scaleList"
		SetupScaleControllers tmListBox scaleListBox
		
		
		-- KZ, do not test with sequential off because thats not how scale_list works.
		-- Compare the final results
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		scaleListBox.scale.controller.average = false
		CompareResults tmListBox scaleListBox
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		scaleListBox.scale.controller.average = true
		CompareResults tmListBox scaleListBox
		delete tmListBox
		delete scaleListBox
		
		-- KZ do not test the scale_list controller with a parent, it behaves very poorly.
		-- TM_list seems to spit out a decent result. Bottom line is TM list does not work well with rotations.
		-- retest with a parent node
		--format "Set a parent node and retest\n"
		--tmListBox = Box()
		--tmListBox.name = "tmList"
		--scaleListBox = Box()
		--scaleListBox.name = "scaleList"
		--parentTM = sphere()
		--parentTM.name = "parent"
		--tmListBox.parent = parentTM
		--scaleListBox.parent = parentTM
		-- Lastly setup a parent TM so the active tm list control doesn't get a rotation/translation setup on it.
		--parentTM.position = [-10,-20,-30]
		--parentTM.rotation = (eulerToQuat ((eulerAngles -45 10 -80)))
		--parentTM.scale = [0.5, 1, 2]
		--SetupScaleControllers tmListBox scaleListBox
		
		-- Compare the final results again
		--format "Compare again transform_list to scale_list\n"
		--tmListBox.transform.controller.average = false
		--tmListBox.transform.controller.sequential = false
		--tmListBox.transform.controller.weightAgainstPrevious = false
		--scaleListBox.scale.controller.average = false
		--CompareResults tmListBox scaleListBox
		--tmListBox.transform.controller.average = true
		--tmListBox.transform.controller.sequential = true
		--tmListBox.transform.controller.weightAgainstPrevious = true
		--scaleListBox.scale.controller.average = true
		--CompareResults tmListBox scaleListBox
		
		--delete tmListBox
		--delete scaleListBox
		--delete parentTM
		
	),
	
	--TODO: unit test to compare with a PRS that has rot_list and pos_List.
	fn tmListController_RotAndPos = 
	(
		-- Make a tm list control and a rotation list control and compare the results!
		format "tmListController_RotAndPos\n"
		
		format "Setup\n"
		tmListBox = Box()
		tmListBox.name = "tmList"
		rotAndPosListBox = Box()
		rotAndPosListBox.name = "rotAndPosList"
		
		-- transform list
		tmListBox.Transform.controller = transform_list()
		tmListBox.Transform.controller.available.controller = prs()
		
		-- rotation list
		rotAndPosListBox.rotation.controller = Euler_XYZ()
		rotAndPosListBox.rotation.controller = Rotation_List()
		rotAndPosListBox.rotation.controller.available.controller = Euler_XYZ()
		
		--position list
		rotAndPosListBox.position.controller = Position_List()
		rotAndPosListBox.position.controller.available.controller = Position_XYZ()
		
		--link the two position controllers together
		-- ex:
		-- tmListBox : tm_list
		--		-PRS
		--			-pos1 <- same
		--			-rot1 <- same
		--			-scale
		--		-PRS
		--			-pos2 <- same
		--			-rot2 <- same
		--			-scale
		-- transListBox : PRS
		--		-prs
		--			-position_list
		--				-pos1 <- same
		--				-pos2 <- same
		--			-rot_list
		--				-rot1 <- same
		--				-rot2 <- same
		--			-scale
		prs1 = tmListBox.transform.controller.getSubCtrl 1
		prs2 = tmListBox.transform.controller.getSubCtrl 2
		rot1 = rotAndPosListBox.rotation.controller.getSubCtrl 1
		rot2 = rotAndPosListBox.rotation.controller.getSubCtrl 2
		pos1 = rotAndPosListBox.position.controller.getSubCtrl 1
		pos2 = rotAndPosListBox.position.controller.getSubCtrl 2
		prs1.rotation.controller = rot1
		prs2.rotation.controller = rot2
		prs1.position.controller = pos1
		prs2.position.controller = pos2
		
		-- setup and share the same Weight
		-- Putting in here some irregular weights.
		with animate on
		(
			at time 0
			(
				tmListBox.transform.controller.weight = #(100.0, 50.0)
				rotAndPosListBox.rotation.controller.weight = #(100.0, 50.0)
				rotAndPosListBox.position.controller.weight = #(100.0, 50.0)
			)
			at time 100
			(
				tmListBox.transform.controller.weight = #(0.0, 150.0)
				rotAndPosListBox.rotation.controller.weight = #(0.0, 150.0)
				rotAndPosListBox.position.controller.weight = #(0.0, 150.0)
			)
		)
		
		-- Setup some animation on the rotation controllers
		rot1.X_Rotation = -100
		rot1.Y_Rotation = 1
		rot1.Z_Rotation = 50
		rot2.X_Rotation = 100
		rot2.Y_Rotation = 0
		rot2.Z_Rotation = 0
		with animate on
		(
			at time 100
			(
				rot1.X_Rotation = 0
				rot1.Y_Rotation = 0
				rot1.Z_Rotation = 0
				rot2.X_Rotation = -1
				rot2.Y_Rotation = -20
				rot2.Z_Rotation = -300
			)
		)
		
		-- Setup some animation on the pos controllers
		pos1.X_Position = -100
		pos1.Y_Position = 1
		pos1.Z_Position = 50
		pos2.X_Position = 100
		pos2.Y_Position = 0
		pos2.Z_Position = 0
		with animate on
		(
			at time 100
			(
				pos1.X_Position = 0
				pos1.Y_Position = 0
				pos1.Z_Position = 0
				pos2.X_Position = -1
				pos2.Y_Position = -20
				pos2.Z_Position = -300
			)
		)
		
		
		-- Compare the final results
		format "Compare transform_list to rotation_list\n"
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		rotAndPosListBox.rotation.controller.average = false
		rotAndPosListBox.position.controller.average = false
		CompareResults tmListBox rotAndPosListBox
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		rotAndPosListBox.rotation.controller.average = true
		rotAndPosListBox.position.controller.average = true
		CompareResults tmListBox rotAndPosListBox
		
		-- retest with a parent node
		format "Set a parent node and retest\n"
		parentTM = sphere()
		parentTM.name = "parent"
		parentTM.position = [-10,-20,-30]
		parentTM.rotation = (eulerToQuat ((eulerAngles -45 10 -80)))
		--parentTM.scale = [0.5, 1, 2]
		tmListBox.parent = parentTM
		rotAndPosListBox.parent = parentTM
		
		-- Compare the final results again
		format "Compare again transform_list to rotation_list\n"
		tmListBox.transform.controller.average = false
		tmListBox.transform.controller.sequential = false
		tmListBox.transform.controller.weightAgainstPrevious = false
		rotAndPosListBox.rotation.controller.average = false
		rotAndPosListBox.position.controller.average = false
		CompareResults tmListBox rotAndPosListBox
		tmListBox.transform.controller.average = true
		tmListBox.transform.controller.sequential = true
		tmListBox.transform.controller.weightAgainstPrevious = true
		rotAndPosListBox.rotation.controller.average = true
		rotAndPosListBox.position.controller.average = true
		CompareResults tmListBox rotAndPosListBox
		
		delete tmListBox
		delete rotAndPosListBox
		delete parentTM
	),
	
	fn TestListControlInterface listControl subControl1 subControl2 =
	(
		format "Testing %...\n" listControl
		
		assert_true (listControl != undefined)
		assert_true (subControl1 != undefined)
		assert_true (subControl2 != undefined)
		assert_false listControl.indexmode
		
		--count, and weight property
		assert_equal 0 (listControl.count)
		assert_equal 0 (listControl.weight.count)
		listControl.available.controller = subControl1
		assert_equal 1 (listControl.count)
		assert_equal 1 (listControl.weight.count)
		listControl.available.controller = subControl2
		assert_equal 2 (listControl.count)
		assert_equal 2 (listControl.weight.count)
		
		-- getSubCtrl
		assert_equal undefined (listControl.getSubCtrl 0)
		assert_equal subControl1 (listControl.getSubCtrl 1)
		assert_equal subControl2 (listControl.getSubCtrl 2)
		
		--getname, setname
		assert_equal "" (listControl.getname 0)
		-- hard to check the names exactly.
		assert_true ((listControl.getname 1).count > 0)
		assert_true ((listControl.getname 2).count > 0)
		listControl.setname 1 "control1"
		listControl.setname 2 "control2"
		assert_equal "control1" (listControl.getname 1)
		assert_equal "control2" (listControl.getname 2)
		
		-- getSubCtrlWeight
		assert_equal 0.0 (listControl.getSubCtrlWeight 0) --bad index
		assert_equal 1.0 (listControl.getSubCtrlWeight 1)
		assert_equal 1.0 (listControl.getSubCtrlWeight 2)
		assert_equal 0.0 (listControl.getSubCtrlWeight 3) --bad index
		assert_equal 100.0 (listControl.weight[1])
		assert_equal 100.0 (listControl.weight[2])
		listControl.weight[1] = 50.0
		listControl.weight[2] = 123.0
		assert_equal 0.5 (listControl.getSubCtrlWeight 1)
		assert_equal 1.23 (listControl.getSubCtrlWeight 2)
		assert_equal 50.0 (listControl.weight[1])
		assert_equal 123.0 (listControl.weight[2])
		
		--active control: setactive, getactive
		assert_equal 1 (listControl.getactive())
		listControl.setactive 99
		assert_equal 2 (listControl.getactive())
		listControl.setactive -99
		assert_equal 1 (listControl.getactive())
		listControl.setactive 2
		assert_equal 2 (listControl.getactive())
		assert_equal 1 listControl.index --should remain unchanged in weight mode
		listControl.setactive 1
		assert_equal 1 (listControl.getactive())
		assert_equal 1 listControl.index --should remain unchanged in weight mode
		
		--indexmode, change to index mode and check active and index
		listControl.indexmode = true
		assert_true listControl.indexmode
		assert_equal 1 (listControl.getactive())
		assert_equal 1 listControl.index
		listControl.setactive 2 --set active
		assert_equal 2 listControl.index
		assert_equal 2 (listControl.getactive())
		listControl.index = 0 --set index
		assert_equal 1 (listControl.getactive())
		assert_equal 1 listControl.index
		listControl.index = 2 --set index
		assert_equal 2 (listControl.getactive())
		assert_equal 2 listControl.index
		listControl.index = 99 --set bad index, should be clampped
		assert_equal 2 (listControl.getactive())
		assert_equal 2 listControl.index
		listControl.index = -2 --set bad index, should be clampped
		assert_equal 1 (listControl.getactive())
		assert_equal 1 listControl.index

		-- setIndexByName
		assert_false (listControl.setIndexByName "aaa")
		assert_equal 1 listControl.index --no change
		assert_false (listControl.setIndexByName "")
		assert_equal 1 listControl.index --no change
		assert_true (listControl.setIndexByName "2")
		assert_equal 2 listControl.index --match "control2"
		assert_equal 2 (listControl.getactive())
		assert_true (listControl.setIndexByName "1")
		assert_equal 1 listControl.index --match "control1"
		assert_equal 1 (listControl.getactive())
		assert_true (listControl.setIndexByName "2")
		assert_true (listControl.setIndexByName "CONT") --match the first one, case-insensitive
		assert_equal 1 listControl.index --match "control1"
		assert_equal 1 (listControl.getactive())
		-- switch back to weight and this should do nothing to the active control...
		listControl.indexmode = false
		listControl.setactive 2
		listControl.indexmode = true
		-- index mode index should be unaffected
		assert_equal 1 listControl.index
		assert_equal 1 (listControl.getactive())
		
		-- cut paste delete (check active control in both modes
		for i = 0 to 1 do
		(
			--Setup
			if (i == 0) then
			(
				format "weight mode\n"
				listControl.indexmode = false
			)
			else
			(
				format "index mode\n"
				listControl.indexmode = true
			)
			listControl.setactive 2
			assert_equal 2 (listControl.getactive())
			
			-- Manipulate the list and check list and active control
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			listControl.cut 3 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			assert_equal 2 (listControl.getactive())
			listControl.cut 0 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			assert_equal 2 (listControl.getactive())
			-- cut
			listControl.cut 2
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal undefined (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive())
			listControl.paste 1
			assert_equal subControl2 (listControl.getSubCtrl 1)
			assert_equal subControl1 (listControl.getSubCtrl 2)
			-- the active index in both modes should get bumpped
			assert_equal 2 (listControl.getactive())
			
			-- paste
			listControl.setactive 2
			assert_equal 2 (listControl.getactive())
			listControl.cut 1
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal undefined (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive())
			listControl.paste 3 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal undefined (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive())
			listControl.paste 0 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal undefined (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive())
			listControl.paste 2
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive())
			-- delete
			listControl.setactive 2
			assert_equal 2 (listControl.getactive())
			listControl.delete 0 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			assert_equal 2 (listControl.getactive())
			listControl.delete 3 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			assert_equal 2 (listControl.getactive())
			listControl.delete -1 --do nothing
			assert_equal subControl1 (listControl.getSubCtrl 1)
			assert_equal subControl2 (listControl.getSubCtrl 2)
			listControl.delete 1
			assert_equal subControl2 (listControl.getSubCtrl 1)
			assert_equal undefined (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive())
			listControl.delete 1
			assert_equal undefined (listControl.getSubCtrl 1)
			assert_equal undefined (listControl.getSubCtrl 2)
			assert_equal 1 (listControl.getactive()) -- A default of 1
				
			-- Add them back for the next pass
			listControl.available.controller = subControl1
			listControl.available.controller = subControl2
		)
		
		
		-- check boxes
		listControl.indexMode = false
		assert_equal false listControl.average
		listControl.average = true
		assert_equal true listControl.average
		listControl.indexMode = true --average is not used in index mode but still can be checked
		assert_equal true listControl.average
		listControl.average = false
		assert_equal false listControl.average
		-- Only TM_list has this option
		if((superclassof(listControl)) == Matrix3Controller) do
		(
			listControl.indexMode = false
			assert_equal false listControl.sequential
			listControl.sequential = true
			assert_equal true listControl.sequential
			listControl.indexMode = true --average is not used in index mode but still can be checked
			assert_equal true listControl.sequential
			listControl.sequential = false
			assert_equal false listControl.sequential
			
			listControl.indexMode = false
			assert_equal false listControl.weightAgainstPrevious
			listControl.weightAgainstPrevious = true
			assert_equal true listControl.weightAgainstPrevious
			listControl.indexMode = true --average is not used in index mode but still can be checked
			assert_equal true listControl.weightAgainstPrevious
			listControl.weightAgainstPrevious = false
			assert_equal false listControl.weightAgainstPrevious
		)
		
		--check tags
		assert_equal "" listControl.tag
		listControl.tag = "testTag";
		assert_equal "testTag" listControl.tag		
		
		format "Done\n" listControl
	),
	
	fn ListControllers_AllTypes =
	(
		-- Exercise all list controller types
		
		-- Test the commonlist controller functionality
		TestListControlInterface (float_list()) (bezier_float()) (bezier_float())
		TestListControlInterface (point3_list()) (bezier_point3()) (bezier_point3())
		TestListControlInterface (position_list()) (bezier_position()) (bezier_position())
		TestListControlInterface (point4_list()) (bezier_point4()) (bezier_point4())
		TestListControlInterface (rotation_list()) (Euler_XYZ()) (Euler_XYZ())
		TestListControlInterface (scale_list()) (bezier_scale()) (bezier_scale())
		TestListControlInterface (transform_list()) (prs()) (prs())
				
		-- Test get and set value
		-- In both modes:weight and index
		
		-- Command panel open
		--max motion mode
		
		--boxNode = box()
		--select boxNode
		--boxNode.name = "testbox"
		
		--float_list
		--assert_false fl.indexMode
		---- weight mode
		--boxNode.position.controller.X_Position.controller = fl
		--boxNode.position = [99, 0, 0]
		--assert_point3_equal [99, 0, 0] (boxNode.position) tolerance:0.0001
		----index mode
		--fl.IndexMode = true
		--assert_false fl.indexMode
		
		--point3_list
		--position_list
		--point4_list
		--rotation_list
		--scale_list
		--transform_list
	),
	
	tests = 
	#(
		tmListController_Trans,
		tmListController_Rotation,
		tmListController_Scale,
		tmListController_RotAndPos,
		ListControllers_AllTypes
		-- TODO: test save and load scene with stuff in it.
	)
		
)

run_test_fixture ListControl_TestFixture script:(getThisScriptFilename())
