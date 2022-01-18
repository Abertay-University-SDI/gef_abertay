#ifndef _GEF_SONY_CONTROLLER_INPUT_MANAGER_H
#define _GEF_SONY_CONTROLLER_INPUT_MANAGER_H

#include <gef.h>
#include <vector>
#include "graphics/colour.h"
#include "maths/vector2.h"

#define gef_SONY_CTRL_SELECT				(1<<0)
#define gef_SONY_CTRL_L3					(1<<1)
#define gef_SONY_CTRL_R3					(1<<2)
#define gef_SONY_CTRL_START				(1<<3)
#define gef_SONY_CTRL_OPTIONS				(1<<3)
#define gef_SONY_CTRL_UP					(1<<4)
#define gef_SONY_CTRL_RIGHT				(1<<5)
#define gef_SONY_CTRL_DOWN					(1<<6)
#define gef_SONY_CTRL_LEFT					(1<<7)
#define gef_SONY_CTRL_L2					(1<<8)
#define gef_SONY_CTRL_R2					(1<<9)
#define gef_SONY_CTRL_L1					(1<<10)
#define gef_SONY_CTRL_R1					(1<<11)
#define gef_SONY_CTRL_TRIANGLE				(1<<12)
#define gef_SONY_CTRL_CIRCLE				(1<<13)
#define gef_SONY_CTRL_CROSS				(1<<14)
#define gef_SONY_CTRL_SQUARE				(1<<15)
#define gef_SONY_CTRL_PS_LOGO			(1<<16)
#define gef_SONY_CTRL_MIC_BUTTON			(1<<17)
#define gef_SONY_CTRL_TOUCH_PAD			(1<<18)

namespace gef
{
	class Platform;

	struct ControllerOutputData
	{
		enum class MicLedSettings {
			OFF,
			ON,
			PULSE,
		};
		enum class LEDBrightness
		{
			LOW,
			MEDIUM,
			HIGH
		};

		struct PlayerLEDs
		{
			PlayerLEDs()
			{
				player_led_fade = false;
				brightness = LEDBrightness::MEDIUM;
			}
			bool player_led_fade;
			LEDBrightness brightness;

		};
		enum class TriggerEffectType : unsigned char {
			/// No resistance is applied
			NoResitance = 0x00,
			/// Continuous Resitance is applied
			ContinuousResitance = 0x01,
			/// Seciton resistance is appleyed
			SectionResitance = 0x02,
			/// Extended trigger effect
			EffectEx = 0x26,
			/// Calibrate triggers
			Calibrate = 0xFC,
		};

		struct TriggerEffect
		{
			TriggerEffect()
			{
				effectType = TriggerEffectType::NoResitance;
				
			}
			TriggerEffectType effectType;
			
			/// Union for effect parameters
			union
			{
				
				/// For type == ContinuousResitance
				struct 
				{
					/// Start position of resistance [0.0f -> 1.0f]
					float startPosition;
					/// Force of resistance [0.0f -> 1.0f]
					float force;
				} Continuous;
				
				/// For type == SectionResitance
				struct 
				{
					/// Start position of resistance [0.0f -> 1.0f]
					float startPosition;
					
					/// End position of resistance (>= start) [start -> 1.0f]
					float endPosition;
					
				} Section;
				
				/// For type == EffectEx
				struct 
				{
					/// Position at witch the effect starts [0.0f -> 1.0f]
					float startPosition=0;
					
					/// Wher the effect should keep playing when trigger goes beyond 1
					bool keepEffect=false;
					
					/// Force applied when trigger >= (0.5)
					float beginForce=0;
					
					/// Force applied when trigger <= (0.5)
					float middleForce=0;
					
					/// Force applied when trigger is beyond 1
					float endForce=0;
					
					/// Vibration frequency of the trigger [0.0f -> 1.0f]
					float frequency=0;
				} EffectEx;
			};
		};


		ControllerOutputData()
		{
			left_rumble = 0;
			right_rumble = 0;
			mic_led = MicLedSettings::OFF;
			disable_leds = false;
			lightbar.SetFromRGBA(0x000000FF);
			
			
		}
		
		/// Left(hard) and Right(soft) Rumble 0.0f -> 1.0f		
		float left_rumble, right_rumble;

		MicLedSettings mic_led;

		bool disable_leds;
		PlayerLEDs player_leds;

		Colour lightbar;

		TriggerEffect left_trigger_effect, right_trigger_effect;



	};

	class SonyController
	{
	public:
		SonyController();
		void UpdateButtonStates(UInt32 previous_buttons_down);

		/// @return the buttons that are currently held down
		inline UInt32 buttons_down() const { return buttons_down_; }
		/// @return the buttons that have been pressed this update
		inline UInt32 buttons_pressed() const { return buttons_pressed_; }
		/// @return the buttons that have been released this update
		inline UInt32 buttons_released() const { return buttons_released_; }

		/// @return the horizontal position of the left analog stick [ -1 <-- left, 1 --> right]
		inline float left_stick_x_axis() const { return left_stick_x_axis_; }
		/// @return the vertical position of the left analog stick [ -1 <-- up, 1 --> down]
		inline float left_stick_y_axis() const { return left_stick_y_axis_; }
		/// @return the horizontal position of the right analog stick [ -1 <-- left, 1 --> right]
		inline float right_stick_x_axis() const { return right_stick_x_axis_; }
		/// @return the vertical position of the right analog stick [ -1 <-- up, 1 --> down]
		inline float right_stick_y_axis() const { return right_stick_y_axis_; }


		// @return the L2 float values from 0->1
		inline float get_left_trigger() const { return left_trigger_; }
		// @return the R2 float values from 0->1
		inline float get_right_trigger() const { return right_trigger_; }

		// @return the touch x (0->2000) and y (0->2048) positions as a Vector2
		inline Vector2 get_touch_position() const { return Vector2(touch_x_, touch_y_); }
		
		// @return the controller output data currently set
		inline ControllerOutputData get_output_data() const { return out_data_; }

		// @return accelerometer values
		inline Vector4 get_accelerometer() const { return accelerometer_; }

		// @return Gyroscope -- Raw values only currently -- needs callibration
		inline Vector4 get_gyroscope() const { return gyropscope_; }
		
		inline void set_buttons_down(const UInt32 buttons_down) { buttons_down_ = buttons_down; }
		inline void set_left_stick_x_axis(const float stick_value) { left_stick_x_axis_ = stick_value; }
		inline void set_left_stick_y_axis(const float stick_value) { left_stick_y_axis_ = stick_value; }
		inline void set_right_stick_x_axis(const float stick_value) { right_stick_x_axis_ = stick_value; }
		inline void set_right_stick_y_axis(const float stick_value) { right_stick_y_axis_ = stick_value; }

		inline void set_left_trigger(const float trigger_value) { left_trigger_ = trigger_value; }
		inline void set_right_trigger(const float trigger_value) { right_trigger_ = trigger_value; }
		inline void set_touch(const int touchx, const int touchy) { touch_x_ = touchx; touch_y_ = touchy; }
		inline void set_accelerometer(const Vector4 data){ accelerometer_ = data; };
		inline void set_gyroscope(const Vector4 data) { gyropscope_ = data; };
		

		inline void set_output_data(const ControllerOutputData& output) const  { const_cast<SonyController*>(this)->out_data_ = output; }

	private:
		UInt32 buttons_down_;
		UInt32 buttons_pressed_;		// Information for pressed buttons
		UInt32 buttons_released_;	// Information for released buttons
		UInt32 previous_buttons_down_;

		float left_stick_x_axis_;
		float left_stick_y_axis_;
		float right_stick_x_axis_;
		float right_stick_y_axis_;
		float left_trigger_;
		float right_trigger_;

		int touch_x_,touch_y_;
		Vector4 accelerometer_, gyropscope_;
		ControllerOutputData out_data_;

	};

	class SonyControllerInputManager
	{
	public:
		SonyControllerInputManager(const Platform& platform);
		virtual ~SonyControllerInputManager();

		/// Update the controller data with the current status of the buttons and analog sticks
		///
		/// @return error status from reading the controllers [0 = success]
		virtual Int32 Update() = 0;
/*
		/// @return the buttons that are currently held down
		inline UInt32 buttons_down() const { return buttons_down_; }
		/// @return the buttons that have been pressed this update
		inline UInt32 buttons_pressed() const { return buttons_pressed_; }
		/// @return the buttons that have been released this update
		inline UInt32 buttons_released() const { return buttons_released_; }

		/// @return the horizontal position of the left analog stick [ -1 <-- left, 1 --> right]
		inline float left_stick_x_axis() const { return left_stick_x_axis_; }
		/// @return the vertical position of the left analog stick [ -1 <-- up, 1 --> down]
		inline float left_stick_y_axis() const { return left_stick_y_axis_; }
		/// @return the horizontal position of the right analog stick [ -1 <-- left, 1 --> right]
		inline float right_stick_x_axis() const { return right_stick_x_axis_; }
		/// @return the vertical position of the right analog stick [ -1 <-- up, 1 --> down]
		inline float right_stick_y_axis() const { return right_stick_y_axis_; }
*/
		inline const SonyController* const GetController(const Int32 controller_num) const
		{
			if (controllersCountDS5 > 0 && controller_num < controllersCountDS5)
				return &DS5_controllers_[controller_num];

			return controller_num == 0 ? &controller_ : 0;
		}

		inline int GetDS5ControllerCount() { return controllersCountDS5; };
	protected:
		SonyController controller_;
		std::vector<SonyController> DS5_controllers_;

		// Num DS5 Controllers count
		unsigned int controllersCountDS5 = 0;
/*
		void UpdateButtonStates();
		UInt32 buttons_down_;
		UInt32 buttons_pressed_;		// Information for pressed buttons
		UInt32 buttons_released_;	// Information for released buttons
		UInt32 previous_buttons_down_;

		float left_stick_x_axis_;
		float left_stick_y_axis_;
		float right_stick_x_axis_;
		float right_stick_y_axis_;

//		static float kStickDeadZone;
*/
	};

}

#endif // _GEF_SONY_CONTROLLER_INPUT_MANAGER_H
