#!/usr/bin/env ruby

# Requires Ruby 1.9 and "gnuplot" gem

require 'gnuplot'
require 'matrix'

EPSILON = 0.000005  # used for solution precision

# boundary conditions
X0 = 0
XN = 1
T0 = 1
TN = 0

PE_VALUES = [0.1, 1, 2, 5, 10, 20, 50]
N_VALUES = [5, 20]


class DivergeException < Exception; end


class Approximator
  def self.build_matrix(n, pe, x0, xn)
    dx = (xn - x0).to_f / n

    left, central, right = self.coefficients(pe, dx, dx * dx)

    Matrix.build(n + 1, n + 1) do |i, j|
      if j == i
        central
      elsif j == i - 1
        left
      elsif j == i + 1
        right
      else
        0
      end
    end
  end

  def self.build_initial_t_vector(n, t0, tn)
    # initially, let's interpolate T linearly between T0
    dt = (tn - t0).to_f / n

    (0..n).collect do |i|
      t0 + i * dt
    end
  end

  def self.approximate_for_pe_and_n(pe, n)
    a = self.build_matrix(n, pe, X0, XN)
    t = self.build_initial_t_vector(n, T0, TN)
    begin
      maxerr = 0
      # using the forward substitution formula from
      # http://en.wikipedia.org/wiki/Gauss%E2%80%93Seidel_method#Description
      # in order not to calculate the boundary points
      te = [t[0]]
      (1..n-1).each do |i|
        sum_old = (i+1..n).inject(0) { |acc, j| acc + t[j] * a[i, j] }
        sum_new = (0..i-1).inject(0) { |acc, j| acc + te[j] * a[i, j] }
        v = -(sum_old + sum_new) / a[i, i]
        raise DivergeException if v.nan?
        te << v
        err = (t[i] - te[i]).abs
        maxerr = err if (err > maxerr)
      end
      te << t[n]
      t = te
    end while maxerr >= EPSILON
    t
  end
end

class CentralCentral < Approximator
  def self.coefficients(pe, dx, dx2)
    left = 1 / (pe * dx2) + 1 / (2 * dx)
    central = -2 / (pe * dx2)
    right = 1 / (pe * dx2) - 1 / (2 * dx)
    [left, central, right]
  end
end

class ForwardCentral < Approximator
  def self.coefficients(pe, dx, dx2)
    left = 1 / (pe * dx2)
    central = 1 / dx - 2 / (pe * dx2)
    right = 1 / (pe * dx2) - 1 / dx
    [left, central, right]
  end
end






Gnuplot.open do |gp|
  N_VALUES.each do |n|
    Gnuplot::Plot.new(gp) do |plot|
      plot.title  "Approximation at n=#{n}"
      plot.ylabel 'T'
      plot.xlabel 'x'
      plot.key 'bottom left'
      plot.size 'ratio 1'
      plot.terminal 'pdf'
      plot.output "5n#{n}.pdf"

      xs = []
      dx = (XN - X0).to_f / n
      (n + 1).times do |i|
        xs << X0 + dx * i
      end
      PE_VALUES.each do |pe|
        begin
          ts = CentralCentral.approximate_for_pe_and_n(pe, n)
          plot.data << Gnuplot::DataSet.new([xs, ts]) do |ds|
            ds.with = "lines"
            ds.title = "Pe=#{pe}"
          end
        rescue DivergeException => ex
          plot.data << Gnuplot::DataSet.new([[0], [0]]) do |ds|
            ds.with = "lines"
            ds.title = "Pe=#{pe} (diverged)"
          end
        end
      end
    end

    Gnuplot::Plot.new(gp) do |plot|
      n = N_VALUES[0]
      plot.title  "Numeric diffusion at n=#{n}"
      plot.ylabel 'numeric diffusion'
      plot.xlabel 'x'
      plot.key 'bottom right'
      plot.size 'ratio 1'
      plot.terminal 'pdf'
      plot.output "5n#{n}d.pdf"
      xs = []
      dx = (XN - X0).to_f / n
      (n + 1).times do |i|
        xs << X0 + dx * i
      end
      PE_VALUES.each do |pe|
        begin
          tscc = CentralCentral.approximate_for_pe_and_n(pe, n)
          tsfc = ForwardCentral.approximate_for_pe_and_n(pe, n)
          ts = (0..n).map { |i| tscc[i] - tsfc[i] }
          plot.data << Gnuplot::DataSet.new([xs, ts]) do |ds|
            ds.with = "lines"
            ds.title = "Pe=#{pe}"
          end
        rescue DivergeException => ex
          plot.data << Gnuplot::DataSet.new([[0], [0]]) do |ds|
            ds.with = "lines"
            ds.title = "Pe=#{pe} (diverged)"
          end
        end
      end
    end
  end
end
