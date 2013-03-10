
close all;


%% Test Speex
qual = 3;
ratio = 2;

impulse_speex = wavread(sprintf('impulse_speex_q%u_%u.wav', qual, ratio))' / ratio;
impulse_len_speex = filter_taps(impulse_speex) / ratio;

alias_speex = wavread(sprintf('alias_speex_q%u_%u.wav', qual, ratio))';
alias_down_speex = wavread(sprintf('alias_down_speex_q%u_%u.wav', qual, ratio))';

plot_responses(impulse_speex, alias_speex, alias_down_speex, ratio);

%% Test SRC
qual = 'fastest';
ratio = 2;

impulse_src = wavread(sprintf('impulse_src_%s_%u.wav', qual, ratio))' / ratio;
impulse_len_src = filter_taps(impulse_src) / ratio;

alias_src = wavread(sprintf('alias_src_%s_%u.wav', qual, ratio))';
alias_down_src = wavread(sprintf('alias_down_src_%s_%u.wav', qual, ratio))';

plot_responses(impulse_src, alias_src, alias_down_src, ratio);


%% Test libavresample
ratio = 2;

impulse_avr = wavread(sprintf('impulse_avr_%u.wav', ratio))' / ratio;
impulse_len_avr = filter_taps(impulse_avr) / ratio;

alias_avr = wavread(sprintf('alias_avr_%u.wav', ratio))';
alias_down_avr = wavread(sprintf('alias_down_avr_%u.wav', ratio))';

plot_responses(impulse_avr, alias_avr, alias_down_avr, ratio);


%% Test libswresample
type = 'soxr';
ratio = 2;

impulse_swr = wavread(sprintf('impulse_swr_%s_%u.wav', type, ratio))' / ratio;
impulse_len_swr = filter_taps(impulse_swr) / ratio;

alias_swr = wavread(sprintf('alias_swr_%s_%u.wav', type, ratio))';
alias_down_swr = wavread(sprintf('alias_swr_%s_%u.wav', type, ratio))';

plot_responses(impulse_swr, alias_swr, alias_down_swr, ratio);

%% Test SSRC
ratio = 2;

impulse_ssrc = wavread(sprintf('impulse_ssrc_%u.wav', ratio))' / ratio;
impulse_len_ssrc = filter_taps(impulse_ssrc) / ratio;

alias_ssrc = wavread(sprintf('alias_ssrc_%u.wav', ratio))';
alias_down_ssrc = wavread(sprintf('alias_down_ssrc_%u.wav', ratio))';

plot_responses(impulse_ssrc, alias_ssrc, alias_down_ssrc, ratio);

