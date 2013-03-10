function plot_responses(impulse, upsampled, downsampled, ratio)

% Impulse response
figure('name', sprintf('Impulse ratio: %u', ratio));
freqz(impulse, 1, 8092);
xlim([0.75 / ratio, 1.25 / ratio]);
title(sprintf('Impulse, Resampling ratio = %.2f\n', ratio));

% Upsample alias
figure('name', sprintf('Upsample ratio: %u', ratio));
[ww, freqs] = freqz(upsampled .* kaiser(length(upsampled), 20.0)', 1, 32 * 1024);
ww_low = ww(1 : end / ratio);
ww_high = ww(end / ratio : end);
freqs_low = freqs(1 : end / ratio);
freqs_high = freqs(end / ratio : end);
hold on;
plot(freqs_low / pi, 20 * log10(abs(ww_low)), 'b');
plot(freqs_high / pi, 20 * log10(abs(ww_high)), 'r');
hold off;
legend('Signal', 'Alias');
xlim([0.78 1.22] / ratio);
title(sprintf('Upsampling ratio %.2f', ratio));

% Downsample alias
figure('name', sprintf('Downsample ratio: %u', ratio));
[ww, freqs] = freqz(downsampled .* kaiser(length(downsampled), 20.0)', 1, 32 * 1024);
ww_low = ww(1 : round(0.82 * end));
ww_high = ww(round(0.82 * end) : end);
freqs_low = freqs(1 : round(0.82 * end));
freqs_high = freqs(round(0.82 * end) : end);
hold on;
plot(freqs_low / pi, 20 * log10(abs(ww_low)), 'b');
plot(freqs_high / pi, 20 * log10(abs(ww_high)), 'r');
hold off;
legend('Signal', 'Alias');
title(sprintf('Resampling ratio: %.2f', 1 / ratio));

