function taps = filter_taps(impulse)

start_taps = 0;
end_taps = 0;

for i = 1 : length(impulse)
    if impulse(i) ~= 0
        start_taps = i;
        break;
    end
end

for i = fliplr(1 : length(impulse))
    if impulse(i) ~= 0
        end_taps = i;
        break;
    end
end

taps = end_taps - start_taps + 1;