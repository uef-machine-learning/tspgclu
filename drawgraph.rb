#!/usr/bin/ruby
require "sdl"
require "ripl"
# sudo apt install ruby-sdl
$pointsize = 4 
$pointsize = 7 
# $pointcolor = [200, 200, 200]
$pointcolor = [200, 30, 30]
$pointcolor = [200, 30, 30]
# $linecolor=[200,200,200]
$linecolor = [40, 40, 40]
$bgcolor = [255, 255, 255]
$zoom = 1.0
$focus = [0.5, 0.5]
#$focus = [0.0, 0.0]

def emptyscreen()
  screen = $screen
  screen.draw_rect(0, 0, screen.w, screen.h, $bgcolor, fill = true, alpha = nil)
  screen.flip
end

def mapcoord(p)
  screen = $screen
  #x = p[0]/$xmax*screen.w*$zoom;
  #y = p[1]/$ymax*screen.h*$zoom;

  #x = (p[0]/$xmax-0.5)*$zoom*screen.w+screen.w*(-$focus[0]+1.0)*$zoom;
  #y = screen.h-((p[1]/$ymax-0.5)*$zoom*screen.h+screen.h*(-$focus[1]+1.0))*$zoom;

  ratio = $xmax / $ymax.to_f
  x = (p[0] / $xmax - $focus[0]) * $zoom * screen.w + screen.w / 2
  y = screen.h / 2 - (p[1] / $ymax - $focus[1]) * $zoom * screen.h
  # x = x*ratio

  #*$zoom*screen.w+screen.w*(-$focus[0]+1.0)*$zoom;
  #y = screen.h-((p[1]/$ymax-0.5)*$zoom*screen.h+screen.h*(-$focus[1]+1.0))*$zoom;

  if x > screen.w || y > screen.h || x < 0 || y < 0
    #return [nil,nil]
    return [x, y]
  else
    return [x, y]
  end
end

def draw_point(p)
  screen = $screen
  #x = p[0]/$xmax*screen.w;
  #y = p[1]/$ymax*screen.h;
  #[x,y] = mapcoord(p)
  (x, y) = mapcoord(p)
  if x != nil
    # screen.draw_circle(x, y, $pointsize, $pointcolor, true, false, 128)
    screen.draw_circle(x, y, $pointsize, $pointcolor, true, false, 255)
  end
end

def draw_line(a, b)
  ds = $ds
  screen = $screen
  (x1, y1) = mapcoord($ds[a])
  (x2, y2) = mapcoord($ds[b])
  if x1 != nil && x2 != nil
    # screen.draw_line(x1, y1, x2, y2, $linecolor, true, 64 * 2)
    # https://www.kmc.gr.jp/~ohai/rubysdl_ref_2.en.html#label-225

    screen.draw_line(x1, y1, x2, y2, $linecolor, false, 64)

    # screen.draw_line(x1, y1+1, x2, y2+1, $linecolor, false, 64)
    # screen.draw_line(x1, y1-1, x2, y2-1, $linecolor, false, 64)
    # screen.draw_line(x1+1, y1, x2+1, y2, $linecolor, false, 64)
    # screen.draw_line(x1-1, y1, x2-1, y2, $linecolor, false, 64)

    # for dx in [0,1,-1,2,-2]
    # for dy in [0,1,-1,2,-2]
    for dx in [0, 1, -1]
      for dy in [0, 1, -1]
        screen.draw_line(x1 + dx, y1 + dy, x2 + dx, y2 + dy, $linecolor, false, 64)
      end
    end
  end
end

def render()
  emptyscreen
  for i in 0..($cmap.size - 1)
    numdraw = $cmap[i].size
    # numdraw = 3 if numdraw > 3
    for j in $cmap[i]
      # draw_line($cmap[i][j], $cmap[i][j + 1])
      draw_line(i, j)
    end
  end
  $ds.each { |p| draw_point(p) }
  $screen.flip
end

dsfn = ARGV[0]
graphfn = ARGV[1]

dstxt = IO.readlines(File.expand_path(dsfn))
#dstxt = IO.readlines(File.expand_path('~/Drive/uef/sipu_cbnn/data/birch1.txt'));
ds = dstxt.collect { |row| row.split.collect { |x| x.to_f } }

#cmap = IO.readlines('1dmap.txt').collect{|x|x.to_i};
#cmap = IO.readlines('1dmap.txt_s1').collect{|x|x.to_i};
#cmap = IO.readlines('s1map.txt').collect{|x|x.split.collect{|y| y.to_i}};
#cmap = IO.readlines('nngraph.txt').collect{|x|x.split[2..-1].collect{|y| y.to_i}};

cmap = []
IO.readlines(graphfn).each { |row|
  s = row.split()
  k = s[1].to_i
  id = s[0].to_i
  cmap << s[2..(2 + k - 1)].collect { |y| y.to_i }

  # puts row
  # puts cmap.inspect
  # Ripl.start :binding => binding
}

#Ripl.start :binding => binding

$cmap = cmap

$ds = ds
$xmax = ds.collect { |x| x[0] }.max
$ymax = ds.collect { |x| x[1] }.max

SDL.init(SDL::INIT_VIDEO)
screen = SDL::Screen.open(1024, 1024, 16, SDL::SWSURFACE)
SDL::WM::set_caption $0, $0
$screen = screen

#emptyscreen; screen.draw_circle(100,100,circlesize,[87,87,87],true,false,rand(64)+128);screen.flip
ds.each { |p| draw_point(p) }
screen.flip

#screen.draw_circle(200,100,4,[200,200,200],true,true,rand(64)+128)
# file:///usr/share/doc/ruby-sdl/rubysdl_ref.en.html

#draw_point(ds[0])
#draw_line(1,10)

#for i in 0..(cmap.size-2)
#draw_line(cmap[i],cmap[i+1])
#end
render()

#Ripl.start :binding => binding

#Red=screen.format.map_rgb(255,0,0)
#screen.draw_circle(320,240,220,[140,180,0], true, true)
#screen.draw_line(20,20,300,200,Red,true, rand(64)+128)
#screen.draw_circle(100,100,50,[87,87,87],false,true,rand(64)+128)
#screen.draw_circle(300,170,50,[87,80,0],true, false, rand(64)+128)
#screen.draw_ellipse(320,240,100,200,[200,255,0],false, false, rand(64)+128)

#screen.flip

while true
  while event = SDL::Event.poll
    case event
    when SDL::Event::KeyDown
      if event.sym == SDL::Key::ESCAPE
        exit
      elsif event.sym == SDL::Key::D
        $focus[0] = $focus[0] + 0.1 / $zoom
        render()
      elsif event.sym == SDL::Key::A
        $focus[0] = $focus[0] - 0.1 / $zoom
        render()
      elsif event.sym == SDL::Key::W
        $focus[1] = $focus[1] + 0.1 / $zoom
        render()
      elsif event.sym == SDL::Key::S
        $focus[1] = $focus[1] - 0.1 / $zoom
        render()
      elsif event.sym == SDL::Key::J
        $zoom = $zoom / 1.3
        render()
      elsif event.sym == SDL::Key::K
        $zoom = $zoom * 1.3
        render()
      end
    when SDL::Event::MouseButtonDown
      return
    end
  end
  sleep 0.05
end
