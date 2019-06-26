#!/bin/bash
tmux new-session -d -s foo

tmux send-keys './improved_source.out configs/improved_source.json' 'C-m'

tmux rename-window 'Foo'
tmux select-window -t foo:0
tmux split-window -h

tmux send-keys './improved_sink.out 127.0.0.1:1503 configs/improved_sink.json' 'C-m'
tmux split-window -v -t 0

tmux send-keys './improved_sink.out 127.0.0.1:1503 configs/improved_sink_2.json' 'C-m'
#tmux split-window -v -t 1

tmux -2 attach-session -t foo