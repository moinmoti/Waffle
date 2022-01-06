import plotly.graph_objects as go

height = 1
nodes = []
with open("MPT.csv") as f:
    for i, line in enumerate(f):
        if int(line.split(",")[0]) == height:
            nodes.append(
                (
                    float(line.split(",")[2]),
                    float(line.split(",")[3]),
                    float(line.split(",")[4]),
                    float(line.split(",")[5]),
                    float(line.split(",")[6]),
                )
            )
        # if i == 10000: break

fig = go.Figure()

# Set axes properties
fig.update_xaxes(range=[0, 7], showgrid=False)
fig.update_yaxes(range=[0, 3.5])

# Add shapes
fig.add_shape(type="rect",
    x0=1, y0=1, x1=2, y1=3,
    line=dict(color="RoyalBlue"),
)
fig.add_shape(type="rect",
    x0=3, y0=1, x1=6, y1=2,
    line=dict(
        color="RoyalBlue",
        width=2,
    ),
    fillcolor="LightSkyBlue",
)
fig.update_shapes(dict(xref='x', yref='y'))
# fig.show()


# Create figure and axes

# Create a Rectangle patch


# plt.savefig("Snapshots/MPT-" + str(height) + ".png")
