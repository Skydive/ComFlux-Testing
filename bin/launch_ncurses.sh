#!/bin/bash
tmux new-session -d -s foo

tmux send-keys './ncurses_remote.out configs/improved_source.json' 'C-m'

tmux rename-window 'Foo'
tmux select-window -t foo:0
tmux split-window -h

tmux send-keys './ncurses_block_screen.out 127.0.0.1:1503 configs/ncurses_block_screen.json' 'C-m'

tmux -2 attach-session -t foo