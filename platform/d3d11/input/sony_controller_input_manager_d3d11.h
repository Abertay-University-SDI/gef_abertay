#ifndef _GEF_SONY_CONTROLLER_INPUT_MANAGER_D3D11_H
#define _GEF_SONY_CONTROLLER_INPUT_MANAGER_D3D11_H

#include <input/sony_controller_input_manager.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <dinputd.h>

#include <ds5w.h>
#include <vector>


#define MAX_CONTROLLERS_DS5 4
namespace gef
{
	class PlatformD3D11;

	class SonyControllerInputManagerD3D11 : public SonyControllerInputManager
	{
	public:
		SonyControllerInputManagerD3D11(const PlatformD3D11& platform, LPDIRECTINPUT8 direct_input);
		~SonyControllerInputManagerD3D11();

		static BOOL CALLBACK enumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
		static BOOL CALLBACK enumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );


		Int32 Update();

	private:
		void UpdateController(SonyController& controller, const DIJOYSTATE2& joystate);
		bool InitialiseDS5Controller();

		Int32 UpdateDS5();

		Int32 UpdateDS5ControllerInput(SonyController& controller, DS5W::DS5InputState& inState);
		Int32 GetControllerOutput(SonyController& controller, DS5W::DS5OutputState& outstate);
		void SetTriggerFromController(const ControllerOutputData::TriggerEffect& in_controller , DS5W::TriggerEffect& out_trigger);

		void CleanUp();



		// Context for controller
		std::vector<DS5W::DeviceContext> con_;
		bool is_ds5_enabled_;


		// Array of controller infos
		DS5W::DeviceEnumInfo infos[MAX_CONTROLLERS_DS5];

		LPDIRECTINPUT8			direct_input_;
		LPDIRECTINPUTDEVICE8	joystick_;
		LPDIRECTINPUTDEVICE8	joystick2_;

		static float kStickDeadZone;
	};
}

#endif // _GEF_SONY_CONTROLLER_INPUT_MANAGER_D3D11_H
