#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

enum Game_Mode {
	GM_MENU,
	GM_SOLARSYSTEM,
};

Game_Mode game_mode = GM_MENU;

internal void
simulate_game(Input* input, f32 dt) {
	if (game_mode == GM_MENU) {
		if (pressed(BUTTON_ENTER)) {
			game_mode = GM_SOLARSYSTEM;
		}
	} else if (game_mode == GM_SOLARSYSTEM) {
		
	}
}
