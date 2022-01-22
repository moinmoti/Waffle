using PlotlyJS

height = 1
nodes = []
tolerance = []
open("MPT.csv") do f
    for (i, line) in enumerate(eachline(f))
        vals = map(w -> parse(Float64, w), split(line, ','))
        if Int(vals[1]) == height
            t = round(vals[end]; digits = 2)
            push!(nodes, tuple(vcat(vals[3:6], t)...))
            push!(tolerance, t)
        end
    end
end

maxTol = maximum(tolerance)
minTol = minimum(tolerance)
meanTol = round(sum(tolerance) / length(tolerance); digits = 2)

traces = []
for n in nodes
    alpha = (n[5] - minTol) / (maxTol - minTol)
    push!(traces, scatter(
        x = [n[1], n[1], n[3], n[3], n[1]],
        y = [n[2], n[4], n[4], n[2], n[2]],
        mode = "lines",
        fill = "toself",
        name = "",
        text = string(n[5]),
        fillcolor = string("rgba(255,143,0,", alpha, ")"),
        line = attr(
            color = "black",
            width = 0.25
        )
    ))
end

push!(traces, scatter(
    x = [nothing],
    y = [nothing],
    mode = "markers",
    marker = attr(
        cmin = 0,
        cmax = 1,
        colorscale = [[0, "rgba(255,143,0,0)"], [1, "rgba(255,143,0,1)"]],
        showscale = true,
        colorbar = attr(
            title = "Node tolerance",
            tickvals = [0, meanTol, 1],
            ticktext = [string(minTol, " (Min)"), string(meanTol, " (Mean)"), string(maxTol, " (Max)")],
            ticks = "outside",
        )
    )
))

layout = Layout(
    xaxis = attr(
        range = [-200, 200],
        showgrid = false,
        zeroline = false,
        mirror = true,
        ticks = "outside",
        showline = true,
        linecolor = "black"
    ),
    yaxis = attr(
        range = [-100, 100],
        showgrid = false,
        zeroline = false,
        mirror = true,
        ticks = "outside",
        showline = true,
        linecolor = "black"
    ),
    showlegend = false,
    title = "Tolerance map at height=0 for 1:1 read-write workload",
    xaxis_title = "Longitude",
    yaxis_title = "Latitude",
    font_size = 6,
    plot_bgcolor = "white"
)

p = plot([t for t in traces], layout)

savefig(p, "MPT.png")
