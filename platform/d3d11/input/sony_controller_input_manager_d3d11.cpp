#include <platform/d3d11/input/sony_controller_input_manager_d3d11.h>
#include <platform/d3d11/system/platform_d3d11.h>


namespace gef
{
	float SonyControllerInputManagerD3D11::kStickDeadZone = 0.1f;

	BOOL CALLBACK SonyControllerInputManagerD3D11::enumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pvRef)
	{
		SonyControllerInputManagerD3D11 *context = (SonyControllerInputManagerD3D11 *) pvRef;

		if (0 <= context->direct_input_->CreateDevice(pdidInstance->guidInstance, &context->joystick_, NULL)) {
			return DIENUM_STOP;
		}

		return DIENUM_CONTINUE;
	}

	BOOL CALLBACK SonyControllerInputManagerD3D11::enumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pvRef)
	{
		SonyControllerInputManagerD3D11 *context = (SonyControllerInputManagerD3D11 *) pvRef;

		if (pdidoi->dwType & DIDFT_AXIS) {
			DIPROPRANGE diprg;
			diprg.diph.dwSize = sizeof( DIPROPRANGE );
			diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
			diprg.diph.dwObj = pdidoi->dwType;
			diprg.diph.dwHow = DIPH_BYID;
			diprg.lMin = 0;
			diprg.lMax = 0xff;

			if (0 > context->joystick_->SetProperty(DIPROP_RANGE, &diprg.diph)) {
				return DIENUM_STOP;
			}
		}

		return DIENUM_CONTINUE;
	}

	SonyControllerInputManagerD3D11::SonyControllerInputManagerD3D11(const PlatformD3D11& platform, LPDIRECTINPUT8 direct_input) :
		SonyControllerInputManager(platform),
		direct_input_(direct_input),
		joystick_(NULL),
		joystick2_(NULL),
		is_ds5_enabled_(false)
	{
		is_ds5_enabled_ = InitialiseDS5Controller();

		if(is_ds5_enabled_)
		{
			DS5_controllers_.resize(controllersCountDS5);
			return;
		}
		
		HRESULT hresult = S_OK;

		// Find a Joystick Device
		IDirectInputJoyConfig8* joystick_config = NULL;
		hresult = direct_input_->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&joystick_config);

		if (joystick_config)
		{
			joystick_config->Release();
			joystick_config = NULL;
		}

		hresult =  direct_input_->EnumDevices(DI8DEVCLASS_GAMECTRL, enumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
		if(SUCCEEDED(hresult))
		{
	
			if (joystick_)
			{
				joystick_->SetDataFormat( &c_dfDIJoystick2 );
				hresult = joystick_->EnumObjects( enumObjectsCallback, this, DIDFT_ALL );
			}
		}

		if(FAILED(hresult))
			CleanUp();


	}

	SonyControllerInputManagerD3D11::~SonyControllerInputManagerD3D11()
	{
		CleanUp();

	}

	void SonyControllerInputManagerD3D11::CleanUp()
	{
		ReleaseNull(joystick_);

		if(is_ds5_enabled_)
		{
			// Shutdown context
			for (int i = 0; i < controllersCountDS5; i++) 
			{
				// Create struct and zero it
				DS5W::DS5OutputState outState;
				ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));
				// Send output to the controller
				DS5W::setDeviceOutputState(&con_[i], &outState);

				DS5W::freeDeviceContext(&con_[i]);
			}
		}
	}


	Int32 SonyControllerInputManagerD3D11::Update()
	{
		if(is_ds5_enabled_)
		{
			return UpdateDS5();
		}

		if (joystick_==NULL) {
			return -1;
		}
		joystick_->Acquire();
		int ret = joystick_->Poll();
		if (ret < 0) {
			return ret;
		}

		DIJOYSTATE2 joystate;
		ret = joystick_->GetDeviceState(sizeof(DIJOYSTATE2), &joystate);
		if (ret < 0) {
			return ret;
		}

		
		UpdateController(controller_, joystate);
		

		return 0;
	}

	void SonyControllerInputManagerD3D11::UpdateController(SonyController& controller, const DIJOYSTATE2& joystate)
	{
		UInt32 previous_buttons_down;

		// get the buttons status before they are updated
		previous_buttons_down = controller.buttons_down();
		UInt32 buttons_down = 0;


		float left_stick_x_axis = (float)joystate.lX;
		float left_stick_y_axis = (float)joystate.lY;
		float right_stick_x_axis = (float)joystate.lZ;
		float right_stick_y_axis = (float)joystate.lRz;

		float left_trigger = (float)joystate.lRx;
		float right_trigger = (float)joystate.lRy;


		if (0x00 != ( 0x80 & joystate.rgbButtons[9])) {
			buttons_down |= gef_SONY_CTRL_SELECT;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[8])) {
			buttons_down |= gef_SONY_CTRL_START;
		}
		if (31500 == joystate.rgdwPOV[0] ||     0 == joystate.rgdwPOV[0] ||  4500 == joystate.rgdwPOV[0]) {
			buttons_down |= gef_SONY_CTRL_UP;
		}
		if ( 4500 == joystate.rgdwPOV[0] ||  9000 == joystate.rgdwPOV[0] || 13500 == joystate.rgdwPOV[0]) {
			buttons_down |= gef_SONY_CTRL_RIGHT;
		}
		if (13500 == joystate.rgdwPOV[0] || 18000 == joystate.rgdwPOV[0] || 22500 == joystate.rgdwPOV[0]) {
			buttons_down |= gef_SONY_CTRL_DOWN;
		}
		if (22500 == joystate.rgdwPOV[0] || 27000 == joystate.rgdwPOV[0] || 31500 == joystate.rgdwPOV[0]) {
			buttons_down |= gef_SONY_CTRL_LEFT;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[4])) {
			buttons_down |= gef_SONY_CTRL_L1;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[5])) {
			buttons_down |= gef_SONY_CTRL_R1;
		}

		if (0x00 != ( 0x80 & joystate.rgbButtons[6])) {
			buttons_down |= gef_SONY_CTRL_L2;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[7])) {
			buttons_down |= gef_SONY_CTRL_R2;
		}

		if (0x00 != ( 0x80 & joystate.rgbButtons[10])) {
			buttons_down |= gef_SONY_CTRL_L3;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[11])) {
			buttons_down |= gef_SONY_CTRL_R3;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[13])) {
			buttons_down |= gef_SONY_CTRL_TOUCH_PAD;
		}

		if (0x00 != ( 0x80 & joystate.rgbButtons[0] )) {
			buttons_down |= gef_SONY_CTRL_SQUARE;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[1] )) {
			buttons_down |= gef_SONY_CTRL_CROSS;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[2] )) {
			buttons_down |= gef_SONY_CTRL_CIRCLE;
		}
		if (0x00 != ( 0x80 & joystate.rgbButtons[3] )) {
			buttons_down |= gef_SONY_CTRL_TRIANGLE;
		}

		controller.set_buttons_down(buttons_down);
		controller.UpdateButtonStates(previous_buttons_down);

		// calculate the stick values
		// -1 to 1 x-axis left to right
		// -1 to 1 y-axis up to down
		left_stick_x_axis = (static_cast<float>(joystate.lX) - 127.5f) / 127.5f;
		left_stick_y_axis = (static_cast<float>(joystate.lY) - 127.5f) / 127.5f;
		right_stick_x_axis = (static_cast<float>(joystate.lZ) - 127.5f) / 127.5f;
		right_stick_y_axis = (static_cast<float>(joystate.lRz) - 127.5f) / 127.5f;

		left_trigger = (static_cast<float>(joystate.lRx) / 255.0);
		right_trigger = (static_cast<float>(joystate.lRy) / 255.0);
		

		// if any of the stick values are less than the dead zone threshold then zero them out
		if(fabsf(left_stick_x_axis) < kStickDeadZone)
			left_stick_x_axis = 0.0f;
		if(fabsf(left_stick_y_axis) < kStickDeadZone)
			left_stick_y_axis = 0.0f;
		if(fabsf(right_stick_x_axis) < kStickDeadZone)
			right_stick_x_axis = 0.0f;
		if(fabsf(right_stick_y_axis) < kStickDeadZone)
			right_stick_y_axis = 0.0f;


		controller.set_left_stick_x_axis(left_stick_x_axis);
		controller.set_left_stick_y_axis(left_stick_y_axis);
		controller.set_right_stick_x_axis(right_stick_x_axis);
		controller.set_right_stick_y_axis(right_stick_y_axis);

		controller.set_left_trigger(left_trigger);
		controller.set_right_trigger(right_trigger);
	}

	bool SonyControllerInputManagerD3D11::InitialiseDS5Controller()
	{
		
		

		// Call enumerate function and switch on return value
		switch (DS5W::enumDevices(infos, MAX_CONTROLLERS_DS5, &controllersCountDS5)) {
		case DS5W_OK:
			// The buffer was not big enough. Ignore for now
		case DS5W_E_INSUFFICIENT_BUFFER:
			break;

			// Any other error will terminate the application
		default:
			// Insert your error handling
			return false;
		}

		// Check number of controllers
		if (controllersCountDS5 == 0)
			return false;

		con_.resize(controllersCountDS5);
		// Init controller and close application is failed
		for (int i = 0; i < controllersCountDS5 && i<MAX_CONTROLLERS_DS5; i++) 
		{
			if (DS5W_FAILED(DS5W::initDeviceContext(&infos[i], &con_[i]))) 
			{
				return false;
			}
		}
		// Return zero
		return true;
	
	}

	Int32 SonyControllerInputManagerD3D11::UpdateDS5()
	{
		// Input state


		for (int i = 0; i < controllersCountDS5; i++)
		{
			
			DS5W::DS5InputState inState;
			// Retrieve data
			if (DS5W_SUCCESS(DS5W::getDeviceInputState(&con_[i], &inState))) 
			{
				
				UpdateDS5ControllerInput(DS5_controllers_[i], inState);

				// Create struct and zero it
				DS5W::DS5OutputState outState;
				ZeroMemory(&outState, sizeof(DS5W::DS5OutputState));

				// Set output data
				outState.playerLeds.bitmask = i==0? DS5W_OSTATE_PLAYER_LED_MIDDLE: i==1? DS5W_OSTATE_PLAYER_LED_MIDDLE_LEFT: i == 2? DS5W_OSTATE_PLAYER_LED_LEFT| DS5W_OSTATE_PLAYER_LED_MIDDLE| DS5W_OSTATE_PLAYER_LED_RIGHT: 0;

				GetControllerOutput(DS5_controllers_[i], outState);

				/*outState.rightTriggerEffect.effectType = DS5W::_TriggerEffectType::EffectEx;
				outState.rightTriggerEffect.EffectEx.startPosition = 10;

				outState.rightTriggerEffect.EffectEx.keepEffect= true;

				outState.rightTriggerEffect.EffectEx.beginForce = 10;
				outState.rightTriggerEffect.EffectEx.middleForce = 100;
				outState.rightTriggerEffect.EffectEx.endForce= 255;*/

				//outState.rightTriggerEffect.effectType = DS5W::_TriggerEffectType::SectionResitance;
				//outState.rightTriggerEffect.Section.startPosition = 00;
				//outState.rightTriggerEffect.Section.endPosition = 100;




				// Send output to the controller
				DS5W::setDeviceOutputState(&con_[i], &outState);
			}
		}
		return 0;
	}

	Int32 SonyControllerInputManagerD3D11::UpdateDS5ControllerInput(SonyController& controller, DS5W::DS5InputState& inState)
	{

		UInt32 previous_buttons_down;

		// get the buttons status before they are updated
		previous_buttons_down = controller.buttons_down();
		UInt32 buttons_down = 0;


		float left_stick_x_axis =  (float)inState.leftStick.x;
		float left_stick_y_axis =  (float)inState.leftStick.y;
		float right_stick_x_axis = (float)inState.rightStick.x;
		float right_stick_y_axis = (float)inState.rightStick.y;

		float left_trigger = (float)inState.leftTrigger;
		float right_trigger = (float)inState.rightTrigger;


		if (inState.buttonsA & DS5W_ISTATE_BTN_A_SELECT) {
			buttons_down |= gef_SONY_CTRL_START;
		}
		if (inState.buttonsA & DS5W_ISTATE_BTN_A_MENU) {
			buttons_down |= gef_SONY_CTRL_SELECT;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_DPAD_UP) {
			buttons_down |= gef_SONY_CTRL_UP;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_DPAD_RIGHT) {
			buttons_down |= gef_SONY_CTRL_RIGHT;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_DPAD_DOWN) {
			buttons_down |= gef_SONY_CTRL_DOWN;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_DPAD_LEFT) {
			buttons_down |= gef_SONY_CTRL_LEFT;
		}
		if (inState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_BUMPER) {
			buttons_down |= gef_SONY_CTRL_L1;
		}
		if (inState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_BUMPER) {
			buttons_down |= gef_SONY_CTRL_R1;
		}

		if (inState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_TRIGGER) {
			buttons_down |= gef_SONY_CTRL_L2;
		}
		if (inState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_TRIGGER) {
			buttons_down |= gef_SONY_CTRL_R2;
		}

		if (inState.buttonsA & DS5W_ISTATE_BTN_A_LEFT_STICK) {
			buttons_down |= gef_SONY_CTRL_L3;
		}
		if (inState.buttonsA & DS5W_ISTATE_BTN_A_RIGHT_STICK) {
			buttons_down |= gef_SONY_CTRL_R3;
		}
		if (inState.buttonsB & DS5W_ISTATE_BTN_B_PAD_BUTTON) {
			buttons_down |= gef_SONY_CTRL_TOUCH_PAD;
		}

		if (inState.buttonsAndDpad & DS5W_ISTATE_BTX_SQUARE) {
			buttons_down |= gef_SONY_CTRL_SQUARE;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_BTX_CROSS) {
			buttons_down |= gef_SONY_CTRL_CROSS;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_BTX_CIRCLE) {
			buttons_down |= gef_SONY_CTRL_CIRCLE;
		}
		if (inState.buttonsAndDpad & DS5W_ISTATE_BTX_TRIANGLE) {
			buttons_down |= gef_SONY_CTRL_TRIANGLE;
		}
		if (inState.buttonsB & DS5W_ISTATE_BTN_B_PLAYSTATION_LOGO) {
			buttons_down |= gef_SONY_CTRL_PS_LOGO;
		}
		if (inState.buttonsB & DS5W_ISTATE_BTN_B_MIC_BUTTON) {
			buttons_down |= gef_SONY_CTRL_MIC_BUTTON;
		}


		controller.set_buttons_down(buttons_down);
		controller.UpdateButtonStates(previous_buttons_down);

		// calculate the stick values
		// -1 to 1 x-axis left to right
		// -1 to 1 y-axis up to down
		left_stick_x_axis =  (static_cast<float>(inState.leftStick.x) ) / 127.f;
		left_stick_y_axis = (static_cast<float>(inState.leftStick.y) ) / 127.f;
		right_stick_x_axis = (static_cast<float>(inState.rightStick.x) ) / 127.f;
		right_stick_y_axis = (static_cast<float>(inState.rightStick.y) ) / 127.f;

		left_trigger = (static_cast<float>(inState.leftTrigger) / 256.0);
		right_trigger = (static_cast<float>(inState.rightTrigger) / 256.0);


		// if any of the stick values are less than the dead zone threshold then zero them out
		if (fabsf(left_stick_x_axis) < kStickDeadZone)
			left_stick_x_axis = 0.0f;
		if (fabsf(left_stick_y_axis) < kStickDeadZone)
			left_stick_y_axis = 0.0f;
		if (fabsf(right_stick_x_axis) < kStickDeadZone)
			right_stick_x_axis = 0.0f;
		if (fabsf(right_stick_y_axis) < kStickDeadZone)
			right_stick_y_axis = 0.0f;


		controller.set_left_stick_x_axis(left_stick_x_axis);
		controller.set_left_stick_y_axis(left_stick_y_axis);
		controller.set_right_stick_x_axis(right_stick_x_axis);
		controller.set_right_stick_y_axis(right_stick_y_axis);


		controller.set_left_trigger(left_trigger);
		controller.set_right_trigger(right_trigger);

		controller.set_touch(inState.touchPoint1.x, inState.touchPoint1.y);

		controller.set_accelerometer(Vector4( inState.accelerometer.x,inState.accelerometer.y, inState.accelerometer.z));
		controller.set_gyroscope(Vector4(inState.gyroscope.x, inState.gyroscope.y, inState.gyroscope.z));

		return 0;
	}

	Int32 SonyControllerInputManagerD3D11::GetControllerOutput(SonyController& controller, DS5W::DS5OutputState& outstate)
	{
		auto controller_data = controller.get_output_data();

		outstate.lightbar.r = controller_data.lightbar.r*255;
		outstate.lightbar.g = controller_data.lightbar.g*255;
		outstate.lightbar.b = controller_data.lightbar.b*255;

		outstate.disableLeds = controller_data.disable_leds;

		outstate.leftRumble = controller_data.left_rumble * 255;
		outstate.rightRumble = controller_data.right_rumble * 255;

		switch (controller_data.mic_led)
		{
		case ControllerOutputData::MicLedSettings::OFF:
			outstate.microphoneLed = DS5W::_MicLed::OFF;
			break;


		case ControllerOutputData::MicLedSettings::ON:
			outstate.microphoneLed = DS5W::_MicLed::ON;
			break;


		case ControllerOutputData::MicLedSettings::PULSE:
			outstate.microphoneLed = DS5W::_MicLed::PULSE;
			break;
		}

		switch (controller_data.player_leds.brightness)
		{
		case ControllerOutputData::LEDBrightness::MEDIUM:
			outstate.playerLeds.brightness = DS5W::MEDIUM;
			break;
		case ControllerOutputData::LEDBrightness::HIGH:
			outstate.playerLeds.brightness = DS5W::HIGH;
			break;
		case ControllerOutputData::LEDBrightness::LOW:
			outstate.playerLeds.brightness = DS5W::LOW;
			break;

		}
		outstate.playerLeds.playerLedFade = controller_data.player_leds.player_led_fade;

		SetTriggerFromController(controller_data.left_trigger_effect, outstate.leftTriggerEffect);
		SetTriggerFromController(controller_data.right_trigger_effect, outstate.rightTriggerEffect);

		
		

		return 0;
	}

	void SonyControllerInputManagerD3D11::SetTriggerFromController(const ControllerOutputData::TriggerEffect& in_controller, DS5W::TriggerEffect& out_trigger)
	{
		 out_trigger.effectType = (DS5W::TriggerEffectType)in_controller.effectType;

		 out_trigger.EffectEx.beginForce = in_controller.EffectEx.beginForce * 255;
		 out_trigger.EffectEx.endForce = in_controller.EffectEx.endForce * 255;
		 out_trigger.EffectEx.frequency = in_controller.EffectEx.frequency* 255;
		 out_trigger.EffectEx.keepEffect = in_controller.EffectEx.keepEffect;
		 out_trigger.EffectEx.middleForce= in_controller.EffectEx.middleForce * 255;
		 out_trigger.EffectEx.startPosition = in_controller.EffectEx.startPosition * 255;

		 out_trigger.Continuous.startPosition = in_controller.Continuous.startPosition * 255;
		 out_trigger.Continuous.force = in_controller.Continuous.force* 255;

		out_trigger.Section.startPosition = in_controller.Section.startPosition* 255;
		out_trigger.Section.endPosition = in_controller.Section.endPosition * 255;
		

	}





}




