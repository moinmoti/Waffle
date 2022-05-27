import plotly.graph_objects as go

height = 0
nodes = []
tolerance = []
with open("Snapshots/Waffle.csv") as f:
    for i, line in enumerate(f):
        if int(line.split(",")[0]) == height:
            nodes.append(
                (
                    float(line.split(",")[2]),
                    float(line.split(",")[3]),
                    float(line.split(",")[4]),
                    float(line.split(",")[5]),
                )
            )

fig = go.Figure()

# Set axes properties
fig.update_xaxes(
    range=[-200, 200],
    showgrid=False,
    zeroline=False,
    mirror=True,
    ticks="outside",
    showline=True,
    linecolor="black",
)
fig.update_yaxes(
    range=[-100, 100],
    showgrid=False,
    zeroline=False,
    mirror=True,
    ticks="outside",
    showline=True,
    linecolor="black",
)

perimeter = 0

# Add shapes
for n in nodes:
    fig.add_trace(
        go.Scatter(
            mode="lines",
            x=[n[0], n[0], n[2], n[2], n[0]],
            y=[n[1], n[3], n[3], n[1], n[1]],
            line=dict(color="black", width=0.25),
        )
    )
    perimeter += 2 * ((n[2] - n[0]) + (n[3] - n[1]))

numNodes = len(nodes)
avgLen = perimeter / numNodes
avgCardinality = 1e7 / numNodes
print(
    f"Total Number of Nodes: {numNodes}\nAverage Perimeter: {avgLen:.2f}\nAverage Cardinality: {avgCardinality:.2f}"
)

""" fig.add_trace(
    go.Scatter(
        x=[None],
        y=[None],
        mode="markers",
        marker=dict(
            cmin=0,
            cmax=1,
            colorscale=[[0, "rgba(255,143,0,0)"], [1, "rgba(255,143,0,1)"]],
            showscale=True,
            colorbar=dict(
                title="Node tolerance",
                tickvals=[0, meanTol, 1],
                ticktext=[
                    str(minTol) + " (Min)",
                    str(meanTol) + " (Mean)",
                    str(maxTol) + " (Max)",
                ],
                ticks="outside",
            ),
        ),
    )
) """

# fig.update_shapes(dict(xref='x', yref='y', line=dict(color="black")))
fig.update_layout(
    showlegend=False,
    # title="Tolerance map at height=0 for 1:1 read-write workload",
    xaxis_title="Longitude",
    yaxis_title="Latitude",
    font_size=6,
    plot_bgcolor="white",
)
fig.write_image(file="Snapshots/Waffle.pdf")
