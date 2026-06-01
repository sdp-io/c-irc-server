from matplotlib import pyplot


# Helper function which draws a graph based on the provided x_points
# and saves the results to a .png file in the same tests directory.
def draw_graph(x_points, y_points):
    pyplot.plot(x_points, y_points)
    pyplot.grid(True)

    pyplot.xlabel("Number of Users")
    pyplot.ylabel("Milliseconds")

    pyplot.title("CIRC epoll() Benchmark")

    pyplot.savefig("benchmark_results.png")
