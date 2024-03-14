- самой реализовать триангуляцию Делоне [ссылка на мега статью](https://ianthehenry.com/posts/delaunay/)
- добавить стили карты (цвета, seed и другие настройки для roughjs в саму карту)
- хранить примерно как sqlite

# Generate Maps

`./prog [-opt-name opt]`

`-w` или `--width` [`int`] -- ширина карты в пикселях,
`-h` или `--height` [`int`] -- высота карты в пикселях,




БД

+---------+--------------+
| n_cells | cell_offsets |
+---------+--------------+
| empty                  |
+------------------------+
| cells                  |
+------------------------+

Cell

+------------------------+
| size in bytes          |
+------------------------+

n | 'line' | [line_style] | [x1y1x2y2]+ |
n | 'point' | [point_style] | [xy]+ |
n | 'triangle' | [triangle_style] | [x1y1x2y2x3y3]+ |

line_style: color | bowing | width
point_style: 

polygon | color
line
circle

compound 