// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_InputEvents_h
#define incl_Input_InputEvents_h

#include "EventDataInterface.h"

namespace Input
{
    //! Contains some fo the input events in generic fashion. Does not depend on any one input method
    namespace Events
    {
        // Note: Continous events should go in pairs and in sequence; for every
        //       'start' event (MOVE_FORWARD_PRESSED) there should be an
        //       'end' event (MOVE_FORWARD_RELEASED) and (MOVE_FORWARD_PRESSED == MOVE_FORWARD_RELEASED + 1)
        static const Core::event_id_t KEY_PRESSED = 1;
        static const Core::event_id_t KEY_RELEASED = 2;
        static const Core::event_id_t SCROLL = 3;
        static const Core::event_id_t MOVE_FORWARD_PRESSED = 4;
        static const Core::event_id_t MOVE_FORWARD_RELEASED = 5;
        static const Core::event_id_t MOVE_BACK_PRESSED = 6;
        static const Core::event_id_t MOVE_BACK_RELEASED = 7;
        static const Core::event_id_t MOVE_LEFT_PRESSED = 8;
        static const Core::event_id_t MOVE_LEFT_RELEASED = 9;
        static const Core::event_id_t MOVE_RIGHT_PRESSED = 10;
        static const Core::event_id_t MOVE_RIGHT_RELEASED = 11;


        //! Event for buffered key input. 
        //! Do not use for any continous input such as avatar movement, it will probably cause input lag
        class BufferedKey : public Foundation::EventDataInterface
        {
            BufferedKey();
        public:
            BufferedKey(int code, Core::uint text) : code_(code), text_(text)  {}
            virtual ~BufferedKey() {}
        
            //! OIS::KeyCode, include OISKeyboard.h for the codes
            const int code_;
            //! The key that was pressed as a character
            const Core::uint text_;
        };

        //! Movement along axis
        struct Axis
        {
            //! relative movement
            int rel_;
            //! absolute movement
            int abs_;
        };

        //! input position that contains both relative and absolute movement for single axis
        class SingleAxisMovement : public Foundation::EventDataInterface
        {
        public:
            Axis z_;
        };

        //! input position that contains both relative and absolute movement for several axis
        class Movement : public Foundation::EventDataInterface
        {
        public:            
            Axis x_;
            Axis y_;
            Axis z_;
        };
    }
}

#endif
