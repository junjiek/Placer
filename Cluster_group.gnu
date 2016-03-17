set title " ------Placement Result------ "
set xrange [459:11151]
set yrange [459:11139]
plot "Cluster_group.gnu_cell" with steps, "Cluster_group.gnu_fix" with steps, "Cluster_group.gnu_overfillesBins" with steps, "Cluster_group.gnu_region" with steps


