/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                SINGLE-HEADER C/++ IMGUI LIBRARY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    DESCRIPTION

        This is a single-header library that provides the input
        groundwork for an immediate-mode GUI system. It has
        support for both mouse and keyboard controls. The library
        DOES NOT take care of tasks like rendering or getting
        input from the user (it only interprets input that you
        provide it).

        This library has no dependencies.

    WHAT IS IMMEDIATE MODE GUI?

        Immediate mode GUI is a style of GUI that does not follow
        the retained mode paradigm of GUI development. It is
        best demonstrated with an example:

            // retained mode example
            {
                Button button = init_button(BUTTON_ID,
                                            x, y, w, h,
                                            "Button Text");
                add_button_to_gui(&button);

                while(1) {
                    update_gui();

                    if(gui_button_clicked(BUTTON_ID)) {
                        // the button was clicked
                    }

                    render_gui();
                }
            }

            // immediate mode example
            {
                while(1) {
                    ui_begin();
                    {
                        if(do_button(BUTTON_ID, x, y, w, h,
                                     "Button Text")) {

                            // if this if-block is entered,
                            // the button was clicked.
                            // this block also renders the
                            // button, and if you don't want
                            // the button anymore, just don't
                            // call do_button, and it'll
                            // magically disappear
                        }
                    }
                    ui_end();
                }
            }

        In the above example, you can see a clear centralization
        effort being made by the immediate-mode GUI approach.
        ALL code pertaining to the button, initialization,
        rendering, handling, etc., is in one spot. This is opposed
        to the retained-mode approach, where the programmer must be
        concerned about initialization, updating, handling, and
        rendering separately.

        As you might imagine, this makes designing UIs in code
        pretty trivial, as the logic code for handling UI input
        is identical to the code that adds/removes/renders the
        UI widgets.

    RF_UI UNIQUE FEATURES

        rf_ui has a few unique features that have been really
        useful to me in the past. They have some specific names
        and/or reasoning behind them, so I'll cover them here
        (so you know what they are).

        NOTE: I use "widget" and "element" interchangably, so
              keep that in mind!

      * floating-point IDs
            rf_ui uses floating-point (specifically double-sized)
            IDs for each widget. This is done for a specific
            reason.

            Usually, to avoid large enums of IDs (or the like),
            I #define a GEN_ID macro to generate a unique ID
            that uses the __LINE__ macro to be generated.

            This poses a problem, though, because there are
            times when one wants to create elements in a loop,
            like so:

                for(int i = 0; i < 10; i++) {
                    if(do_button(GEN_ID, x, y+i*h, w, h, "Button")) {
                        // do something
                    }
                }

            The problem is that GEN_ID will generate the same
            ID for all buttons generated by the loop (which
            effectively breaks the input system).

            The advantage in using floating-point IDs is the
            ability to treat the decimal portion of the number
            as a "sub-ID"... like so:

                for(int i = 0; i < 10; i++) {
                    if(do_button(GEN_ID + (i/100.f), x, y+i*h, w, h, "Button")) {

                    }
                }

            Notice how GEN_ID is being added to (i/100.f), which
            is loop variant, to generate the ID for the widget.
            This makes each widget have a unique ID, while also
            not overlapping with IDs for other widgets.

      * ui "focus"
            rf_ui provides the concept of a ui "focus" which is
            implemented to allow super easy keyboard-support.
            Basically, the UI's "focus" is the set of all
            elements in the UI that can be currently navigated
            through using the keyboard (usually with arrowkeys
            or something similar). Any non-focused elements can
            NOT be navigated to using the keyboard.

      * ui "focus" groups
            UI focus also includes the concept of "focus groups".
            These are most useful when, for example, the GUI
            you're designing has multiple columns of widgets.
            It's pretty nice on the end-user to only "focus"
            one column at a time, so they can navigate through
            only one column. They can then switch between these
            columns to change "focus". The "focus group" of the
            UI is the set of elements that are currently focused.

            Different focus groups are designated with different
            numbers. There's a special case: 0. When a UI focus
            has an ID of 0, it means it's always focused, no matter
            the current focus group of the UI.

            Here's an example that'll hopefully make it clearer:

                // focus group: |
                //              |
                //              v
                rf_ui_focus(ui, 1);     // ONLY focused when the focus group is 1
                {
                    if(do_button(...)) {

                    }
                }
                rf_ui_unfocus();

                rf_ui_focus(ui, 2);     // ONLY focused when the focus group is 2
                {
                    if(do_button(...)) {

                    }
                }
                rf_ui_unfocus();

                rf_ui_focus(ui, 0);     // ALWAYS focused
                {
                    if(do_button(...)) {

                    }
                }
                rf_ui_unfocus();

    USAGE

        To use this library, you must #define RF_UI_IMPLEMENTATION
        in ONE .c or .cpp file before #include'ing this file.

        Then, you must create an rf_UIState
        object and initialize it with rf_ui_init:

            rf_UIState ui = rf_ui_init();

        During the main loop of your program, when you want
        to begin allowing UI capabilities, you must call
        rf_ui_begin, passing the rf_UIState you made earlier.

        When you want to end the current loop (with regards
        to UI functionality), you then must call rf_ui_end:

            while(1) {
                // ...

                rf_ui_begin(&ui);
                {
                    // UI stuff goes here
                }
                rf_ui_end(&ui);

                // ...
            }

        The rf_UIState holds variables that track the input
        to be used in the UI. These are input-implementation
        agnostic, so you'll need to set them yourself each
        loop AFTER rf_ui_begin:

            // suppose that I have mouse_x and mouse_y
            // tracking mouse input in screen coordinates.
            // also suppose that I am tracking my two
            // primary mouse buttons with "left_mouse_down"
            // and "right_mouse_down".

            // suppose that key press states are stored in a
            // boolean array called key_press, and
            // key hold states are stored in a boolean array
            // called key_down.

            // I'll let the UI state know about all that.

            rf_ui_begin(&ui);

            ui.cursor_x = mouse_x;
            ui.cursor_y = mouse_y;

            ui.controls[RF_UI_CONTROL_LEFT_MOUSE] = left_mouse_down;
            ui.controls[RF_UI_CONTROL_RIGHT_MOUSE] = right_mouse_down;

            ui.controls[RF_UI_CONTROL_UP_HOLD] = key_down[KEY_UP];
            ui.controls[RF_UI_CONTROL_UP_PRESS] = key_press[KEY_UP];

            ui.controls[RF_UI_CONTROL_LEFT_HOLD] = key_down[KEY_LEFT];
            ui.controls[RF_UI_CONTROL_LEFT_PRESS] = key_press[KEY_LEFT];

            ui.controls[RF_UI_CONTROL_DOWN_HOLD] = key_down[KEY_DOWN];
            ui.controls[RF_UI_CONTROL_DOWN_PRESS] = key_press[KEY_DOWN];

            ui.controls[RF_UI_CONTROL_RIGHT_HOLD] = key_down[KEY_RIGHT];
            ui.controls[RF_UI_CONTROL_RIGHT_PRESS] = key_press[KEY_RIGHT];

            ui.controls[RF_UI_CONTROL_ACTIVATE] = key_press[KEY_ENTER];

            ui.controls[RF_UI_CONTROL_BACKSPACE] = key_press[KEY_BACKSPACE];

            // this might seem cumbersome, but...
            // it is pretty powerful from the library user's perspective,
            // because you can do a lot to control each UI action.

            // for example, if I want Tab and Shift+Tab to also act as
            // navigation:

            ui.controls[RF_UI_CONTROL_UP_PRESS] = key_press[KEY_UP] || (key_down[KEY_SHIFT] && key_press[KEY_TAB]);
            ui.controls[RF_UI_CONTROL_DOWN_PRESS] = key_press[KEY_DOWN] || key_press[KEY_TAB];

        Once you've called rf_ui_begin and set up the input for the loop,
        you're all ready to go! You can call widget functions now.

            if(rf_button(0, 32, 32, 128, 64)) {
                printf("Hello, World!");
            }

        Keep in mind, though, that this is only the INPUT side of
        things. The rendering stuff is intentionally left out to
        allow maximum control over the way things look and behave
        visually.

        I'll be writing a sample front-end for this UI library
        to provide a basic framework for rendering, but I
        think it's pretty important to separate the input side
        from the visual side, because UI USUALLY behaves the
        same across applications (buttons fire when you release
        the mouse, etc.) but one application will want to look
        totally  different from another.

        This is also discounting the vast number
        of visual libraries there are out there; someone might
        be directly using OpenGL, and someone else might be using
        SDL, Allegro5, or SFML. I didn't want to trap anyone!

        The IMGUI nature of the UI can be preserved with a
        front-end attached. In my experience, the front-end
        will naturally mirror the back-end; a front-end might
        have a function do_button that will internally call
        rf_button (and use that to handle input), while also
        doing rendering stuff in the same call.

    DEFAULTLY SUPPORTED WIDGETS

      * Buttons
            Buttons are created/handled using the rf_button
            function. rf_button returns an integer, 1 if the
            button was clicked, and 0 if it was not. It
            takes the following as parameters:

             - a pointer to the rf_UIState with which it should
               be used

             - a (should be) unique floating-point ID

             - x, y coordinates of the button

             - width and height of the button

      * Sliders

            Sliders are created/handled using the rf_slider
            function. rf_slider returns a float in the range
            0-1 that determines how "full" the slider is. It
            takes the following as parameters:

             - a pointer to the rf_UIState with which it should
               be used

             - a (should be) unique floating-point ID

             - x, y coordinates of the slider

             - width and height of the slider

             - a float that holds the current [0-1] value of
               the slider

      * Line Edits

            Line-Edits are created/handled using the rf_line_edit
            function. rf_line_edit returns a char * to the text
            that was passed in. It takes the following as
            parameters:

             - a pointer to the rf_UIState with which it should
               be used

             - a (should be) unique floating-point ID

             - x, y coordinates of the line-edit

             - width and height of the line-edit

             - a char * pointing to a character array to be
               modified by the widget

             - an unsigned int that holds the maximum number
               of characters that the text can hold

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _RF_UI_H
#define _RF_UI_H

#ifndef RF_UI_MAX_ELEMENTS
#define RF_UI_MAX_ELEMENTS 1000
#endif

#define _rf__ui_id_equ(id1, id2) ((int)(id1 * 10000) == (int)(id2 * 10000))
#define _rf__ui_cursor_over(ui, x, y, w, h) (ui->cursor_x >= x && ui->cursor_x <= x+w && ui->cursor_y >= y && ui->cursor_y <= y+h)

typedef double rf_ui_id;

enum {
    RF_UI_CONTROL_LEFT_MOUSE,
    RF_UI_CONTROL_RIGHT_MOUSE,
    RF_UI_CONTROL_UP_PRESS,
    RF_UI_CONTROL_LEFT_PRESS,
    RF_UI_CONTROL_DOWN_PRESS,
    RF_UI_CONTROL_RIGHT_PRESS,
    RF_UI_CONTROL_UP_HOLD,
    RF_UI_CONTROL_LEFT_HOLD,
    RF_UI_CONTROL_DOWN_HOLD,
    RF_UI_CONTROL_RIGHT_HOLD,
    RF_UI_CONTROL_ACTIVATE,
    RF_UI_CONTROL_BACKSPACE,
    RF_MAX_UI_CONTROL
};

typedef struct rf_UIState {
    rf_ui_id hot,
             active;

    rf_ui_id focus_ids[RF_UI_MAX_ELEMENTS];
    unsigned int focus_id_count;
    long int current_focus_id,
             current_focus_group;
    int focusing;

    float cursor_x, cursor_y;
    int controls[RF_MAX_UI_CONTROL];
    char char_input;
} rf_UIState;

rf_UIState rf_ui_init(void);
void rf_ui_begin(rf_UIState *ui);
void rf_ui_end(rf_UIState *ui);
int rf_button(rf_UIState *ui, rf_ui_id id, float x, float y, float w, float h);
float rf_slider(rf_UIState *ui, rf_ui_id id, float x, float y, float w, float h, float value);
char *rf_line_edit(rf_UIState *ui, rf_ui_id id, float x, float y, float w, float h, char *text, unsigned int max_chars);

#ifdef RF_UI_IMPLEMENTATION

unsigned int _rf__ui_strlen(char *s) {
    unsigned int len = 0;
    while(*s++) {
        ++len;
    }
    return len;
}

rf_UIState rf_ui_init(void) {
    rf_UIState ui;
    ui.hot = -1;
    ui.active = -1;

    ui.focus_id_count = 0;
    ui.current_focus_id = -1;
    ui.current_focus_group = 0;
    ui.focusing = 0;

    ui.cursor_x = 0;
    ui.cursor_y = 0;
    for(int i = 0; i < RF_MAX_UI_CONTROL; ++i) {
        ui.controls[i] = 0;
    }
    ui.char_input = 0;
    return ui;
}

void rf_ui_begin(rf_UIState *ui) {
    ui->cursor_x = 0;
    ui->cursor_y = 0;
    for(int i = 0; i < RF_MAX_UI_CONTROL; ++i) {
        ui->controls[i] = 0;
    }
    ui->focus_id_count = 0;
}

void rf_ui_end(rf_UIState *ui) {
    if(ui->current_focus_id < 0) {
        if(ui->focus_id_count &&
           (ui->controls[RF_UI_CONTROL_UP_PRESS] ||
            ui->controls[RF_UI_CONTROL_LEFT_PRESS] ||
            ui->controls[RF_UI_CONTROL_DOWN_PRESS] ||
            ui->controls[RF_UI_CONTROL_RIGHT_PRESS])) {
            ui->current_focus_id = 0;
        }
    }
    else {
        if(ui->focus_id_count) {
            int focus_update = 0;
            if(ui->controls[RF_UI_CONTROL_DOWN_PRESS]) {
                if(++ui->current_focus_id >= ui->focus_id_count) {
                    ui->current_focus_id = 0;
                }
                focus_update = 1;
            }
            if(ui->controls[RF_UI_CONTROL_UP_PRESS]) {
                if(--ui->current_focus_id < 0) {
                    ui->current_focus_id = ui->focus_id_count-1;
                }
                focus_update = 1;
            }

            if(focus_update) {
                if(ui->current_focus_id < 0 || ui->current_focus_id >= ui->focus_id_count) {
                    ui->current_focus_id = 0;
                }
                ui->hot = ui->focus_ids[ui->current_focus_id];
            }
        }
        else {
            ui->current_focus_id = -1;
        }
    }
}

void rf_ui_focus(rf_UIState *ui, long int group) {
    ui->focusing = (ui->current_focus_group == group || !group);
}

void rf_ui_unfocus(rf_UIState *ui) {
    ui->focusing = 0;
}

int rf_button(rf_UIState *ui, rf_ui_id id, float x, float y, float w, float h) {
    int activated = 0;

    if(ui->focusing && ui->focus_id_count < RF_UI_MAX_ELEMENTS) {
        ui->focus_ids[ui->focus_id_count++] = id;
    }

    if(ui->current_focus_id < 0) {
        if(_rf__ui_cursor_over(ui, x, y, w, h)) {
            if(ui->hot < 0) {
                ui->hot = id;
            }
            if(_rf__ui_id_equ(ui->active, id) && !ui->controls[RF_UI_CONTROL_LEFT_MOUSE]) {
                activated = 1;
            }
        }
        else {
            if(_rf__ui_id_equ(ui->hot, id)) {
                ui->hot = -1;
            }
        }

        if(_rf__ui_id_equ(ui->hot, id)) {
            if(ui->controls[RF_UI_CONTROL_LEFT_MOUSE]) {
                ui->active = id;
            }
        }
    }
    else {
        if(_rf__ui_id_equ(ui->hot, id)) {
            if(ui->controls[RF_UI_CONTROL_ACTIVATE]) {
                activated = 1;
            }
        }
    }

    return activated;
}

float rf_slider(rf_UIState *ui, rf_ui_id id, float x, float y, float w, float h, float value) {
    if(ui->focusing && ui->focus_id_count < RF_UI_MAX_ELEMENTS) {
        ui->focus_ids[ui->focus_id_count++] = id;
    }

    if(ui->current_focus_id < 0) {
        if(_rf__ui_id_equ(ui->active, id)) {
            if(ui->controls[RF_UI_CONTROL_LEFT_MOUSE]) {
                value = (ui->cursor_x - x)/w;
            }
            else {
                ui->active = -1;
            }
        }
        else {
            if(_rf__ui_cursor_over(ui, x, y, w, h)) {
                if(ui->hot < 0) {
                    ui->hot = id;
                }
            }
            else {
                if(_rf__ui_id_equ(ui->hot, id)) {
                    ui->hot = -1;
                }
            }

            if(_rf__ui_id_equ(ui->hot, id)) {
                if(ui->controls[RF_UI_CONTROL_LEFT_MOUSE]) {
                    ui->active = id;
                }
            }
        }
    }
    else {
        if(_rf__ui_id_equ(ui->hot, id)) {
            ui->active = id;
            if(ui->controls[RF_UI_CONTROL_RIGHT_HOLD]) {
                if((value += 0.05) > 1) {
                    value = 1;
                }
            }
            if(ui->controls[RF_UI_CONTROL_LEFT_HOLD]) {
                if((value -= 0.05) < 0) {
                    value = 0;
                }
            }
        }
    }

    if(value < 0) {
        value = 0;
    }
    else if(value > 1.f) {
        value = 1.f;
    }

    return value;
}

char *rf_line_edit(rf_UIState *ui, rf_ui_id id, float x, float y, float w, float h, char *text, unsigned int max_chars) {
    unsigned int text_len = _rf__ui_strlen(text);

    if(ui->focusing && ui->focus_id_count < RF_UI_MAX_ELEMENTS) {
        ui->focus_ids[ui->focus_id_count++] = id;
    }

    if(ui->current_focus_id < 0) {
        if(_rf__ui_cursor_over(ui, x, y, w, h)) {
            if(ui->hot < 0) {
                ui->hot = id;
            }
        }
        else {
            if(_rf__ui_id_equ(ui->hot, id)) {
                ui->hot = -1;
            }
        }

        if(_rf__ui_id_equ(ui->hot, id)) {
            if(ui->controls[RF_UI_CONTROL_LEFT_MOUSE]) {
                ui->active = id;
            }
        }
    }
    else {
        if(_rf__ui_id_equ(ui->hot, id)) {
            ui->active = id;
        }
    }

    if(_rf__ui_id_equ(ui->active, id)) {
        if(ui->char_input && text_len < max_chars-1) {
            text[text_len++] = ui->char_input;
        }
        if(ui->controls[RF_UI_CONTROL_BACKSPACE] && text_len > 0) {
            text[--text_len] = 0;
        }
    }

    return text;
}

#endif /* RF_UI_IMPLEMENTATION */

#endif

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MIT License

Copyright (c) 2017 Ryan Fleury

Permission is hereby granted, free of charge, to any
person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without
limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following
conditions: The above copyright notice and this permission
notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
