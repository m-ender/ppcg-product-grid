w = ARGV.shift.to_i
h = ARGV.shift.to_i
eq = ARGV.shift

puts '%d %d' % [w,h]

directions = [[-1,-1 ],
              [ 0,-1 ],
              [ 1,-1 ],
              [ 1, 0 ],
              [ 1, 1 ],
              [ 0, 1 ],
              [-1, 1 ],
              [-1, 0 ]]

characters = ['=', 'x', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9']

grid = ['']*h

done = false
while !done
    grid.fill{|i|'.'*w}
    x = Random.rand(w)
    y = Random.rand(h)

    grid[y][x] = eq[0]

    i = 1
    while i < eq.length
        valid_dirs = directions.select do |dir|
            xt = x+dir[0]
            yt = y+dir[1]
            xt > -1 && xt < w && yt > -1 && yt < h && grid[yt][xt] == '.'
        end
        break if valid_dirs.length == 0

        dir = valid_dirs.sample
        x += dir[0]
        y += dir[1]
        grid[y][x] = eq[i]

        i += 1
    end
    done = i == eq.length
end

grid.each do |row|
    row.chars.each_index do |i|
        row[i] = characters.sample if row[i] == '.'
    end
end

puts grid
