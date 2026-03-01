fileIn "MxsUnit/MxsUnit.ms"

struct Attach_TestFixture
(
	
		
	fn setUp =
	(
		resetMaxFile #noprompt 
	),
	
	fn tearDown =
	(
	),

	
	
	 fn attach = (
        format "\nCreating nodes ..\n"
        -- nodes setup
        bbox = box width:25 length:25 height:50
        ppoint = point()
        if not (assert_defined bbox message:"bbox not defined, test not executed.") then (return false)
        if not (assert_defined ppoint message:"ppoint not defined, test not executed.") then (return false)


        format "Creating Attachment Constraint ..\n"
        -- create attachment constraint
        attach_ctrl = Attachment()
        ppoint.position.controller = attach_ctrl
		 
        if not (assert_string_equal (ppoint.position.controller as string) (Attachment() as string) message:"Attachment() controller not assigned, test not executed.") then (return false)

        format "Setting up Attachment Constraint..\n"
        -- setup constraint
        attach_ctrl.node = bbox
        if not (assert_defined attach_ctrl.node message:"Attachment node not defined, test not executed.") then (return false)
		
		holdMaxFile() --check save with no keys
		fetchMaxFile quiet:true
		attach_ctrl = $Point001.position.controller 
		
        addNewKey attach_ctrl 0f
        attach_key = AttachCtrl.getKey attach_ctrl 1
        attach_key.face = 0 -- this is 0 based, so face ID is actually 4
		
		holdMaxFile() --check save with 1 keys
		fetchMaxFile quiet:true		
		attach_ctrl = $Point001.position.controller 
		bbox = $Box001


        format "Adding Bend modifier on Box..\n"
        -- add bend modifier to box for animation test
        addmodifier bbox (bend())
        if not (assert_defined bbox.modifiers[1] message:"bbox bend modifier not defined, test not executed.") then (return false)

        format "Animating Bend modifier..\n"
        -- setup bend modifier animation
        bbox.modifiers[1].BendAngle.controller = (bezier_float ())
        addnewkey bbox.modifiers[1].BendAngle.controller 50
        addnewkey bbox.modifiers[1].BendAngle.controller 100
        if not (assert_equal 2 bbox.modifiers[1].BendAngle.keys.count message:"Bend modifier Angle not expected key count" ) then (return false)

        bbox.modifiers[1].BendAngle.keys[2].value = 90
        if not (assert_equal 0 bbox.modifiers[1].BendAngle.keys[1].value message:"Bend modifier Angle anim key 1 not expected value" ) then (return false)
        if not (assert_equal 90 bbox.modifiers[1].BendAngle.keys[2].value message:"Bend modifier Angle anim key 2 not expected value" ) then (return false)

        bbox.modifiers[1].BendDir.controller = (bezier_float ())
        addnewkey bbox.modifiers[1].BendDir.controller 50
        addnewkey bbox.modifiers[1].BendDir.controller 100
        if not (assert_equal 2 bbox.modifiers[1].BendDir.keys.count message:"Bend modifier Direction not expected key count" ) then (return false)

        bbox.modifiers[1].BendDir.keys[2].value = 90
        if not (assert_equal 0 bbox.modifiers[1].BendDir.keys[1].value message:"Bend modifier Direction anim key 1 not expected value" ) then (return false)
        if not (assert_equal 90 bbox.modifiers[1].BendDir.keys[2].value message:"Bend modifier Direction anim key 2 not expected value" ) then (return false)
			
		addNewKey attach_ctrl 20f
        attach_key = AttachCtrl.getKey attach_ctrl 2
        attach_key.face = 1 -- this is 0 based, so face ID is actually 4
			
		holdMaxFile() --check save with multiple keys
		fetchMaxFile quiet:true
		attach_ctrl = $Point001.position.controller 
		bbox = $Box001


		addNewKey attach_ctrl 50f
        attach_key = AttachCtrl.getKey attach_ctrl 3
        attach_key.face = 2 -- this is 0 based, so face ID is actually 4
		attach_ctrl = $Point001.position.controller 
		bbox = $Box001

        key = AttachCtrl.getKey attach_ctrl 1
        assert_equal key.time 0

		key = AttachCtrl.getKey attach_ctrl 2
        assert_equal key.time 20
		
        key = AttachCtrl.getKey attach_ctrl 3
        assert_equal key.time 50
		
		selectKey attach_ctrl 2
		deleteKeys attach_ctrl #selection
		
		key = AttachCtrl.getKey attach_ctrl 1
        assert_equal key.time 0

		
        key = AttachCtrl.getKey attach_ctrl 2
        assert_equal key.time 50
		
		addNewKey attach_ctrl 10f
		
		
        key = AttachCtrl.getKey attach_ctrl 1
        assert_equal key.time 0

		key = AttachCtrl.getKey attach_ctrl 2
        assert_equal key.time 10
		
        key = AttachCtrl.getKey attach_ctrl 3
        assert_equal key.time 50


		addNewKey attach_ctrl 10f
		max motion mode
		slidertime = 10
		select $Point001
		
		numK = numKeys attach_ctrl
		assert_equal 3 numK
		
        key = AttachCtrl.getKey attach_ctrl 1
        assert_equal 0 key.time 

		key = AttachCtrl.getKey attach_ctrl 2
        assert_equal 10 key.time 
		
        key = AttachCtrl.getKey attach_ctrl 3
        assert_equal 50 key.time
		
		holdMaxFile() --check save with multiple keys
		fetchMaxFile quiet:true
		attach_ctrl = $Point001.position.controller 
		bbox = $Box001
		
		numK = numKeys attach_ctrl
		assert_equal 3 numK
		
        key = AttachCtrl.getKey attach_ctrl 1
        assert_equal 0 key.time 

		key = AttachCtrl.getKey attach_ctrl 2
        assert_equal 10 key.time 
		
        key = AttachCtrl.getKey attach_ctrl 3
        assert_equal 50 key.time
		
		
		--try to simulate setting key or dragging key
		--neither work
		--for instance this wont work since keys wont get resorted
		key.time  = 5 --LEFGACY DEFECT this will not update the key frame table correctly
		slidertime = 10
		select $Point001		
		slidertime = 0

		--so for now use save/load to clear dupes and sorts
		holdMaxFile() --check save with multiple keys
		fetchMaxFile quiet:true
		attach_ctrl = $Point001.position.controller 
		bbox = $Box001

        key = AttachCtrl.getKey attach_ctrl 1
        assert_equal 0 key.time 

		key = AttachCtrl.getKey attach_ctrl 2
        assert_equal 5 key.time 
		
        key = AttachCtrl.getKey attach_ctrl 3
        assert_equal 10 key.time

/*
hmm movekey does not work 
no way to simulate a drag
		selectKey attach_ctrl 3
		moveKeys attach_ctrl 10 #selection  


        key = AttachCtrl.getKey attach_ctrl 1
        assert_equal key.time 0

		key = AttachCtrl.getKey attach_ctrl 2
        assert_equal  10 key.time
		
        key = AttachCtrl.getKey attach_ctrl 3
        assert_equal  20 key.time
		
		--LEFGACT DEFECT this will not update the key frame table correctly
		--get key past frame 50 is not updating
		
		silderTime = 50
		p1 = $Point001.pos

		silderTime = 100
		p2 = $Point001.pos
		assert_not_equal p1 p2
			
		holdMaxFile() 
		fetchMaxFile quiet:true
*/
			

        return true
    ),
	
	tests = 
	#(
		attach
	)
		
)

run_test_fixture Attach_TestFixture script:(getThisScriptFilename())
