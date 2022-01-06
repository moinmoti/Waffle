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
                    round(float(line.split(",")[6]), 2),
                )
            )
        # if i == 10000: break

fig = go.Figure()

# Set axes properties
fig.update_xaxes(range=[-200, 200], showgrid=False, zeroline=False)
fig.update_yaxes(range=[-100, 100], showgrid=False, zeroline=False)

# Add shapes
colors = []
for n in nodes:
    # fig.add_shape(type="rect",
    #     x0=n[0], y0=n[1], x1=n[2], y1=n[3],
    #     fillcolor="green",
    #     opacity=n[4],
    #     text=str(n[4]),
    # )
    fig.add_trace(
        go.Scatter(
            mode="lines",
            x=[n[0],n[0],n[2],n[2],n[0]],
            y=[n[1],n[3],n[3],n[1],n[1]],
            fill="toself",
            name="",
            text=str(n[4]),
            fillcolor='rgba(0,0,0,'+str(n[4])+')',
            line=dict(
                color="black", width=1),
        )
    )

fig.add_trace(
    go.Scatter(
        x=[None],
        y=[None],
        mode='markers',
        marker=dict(
            cmin=0,
            cmax=1,
            colorscale=[[0,'rgba(0,0,0,0)'],[1, 'rgba(0,0,0,1)']],
            showscale=True,
            colorbar=dict(
                title="Node tolerance",
                tickvals=[0, 0.25, 0.5, 0.75, 1],
            )
        )
    )
)

# fig.update_shapes(dict(xref='x', yref='y', line=dict(color="black")))
fig.update_layout(showlegend=False)
fig.show()


# Create figure and axes

# Create a Rectangle patch


# plt.savefig("Snapshots/MPT-" + str(height) + ".png")
