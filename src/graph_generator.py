import graphviz
splines = input("Do you want splines in your graph? (y/n) ")

if(splines == 'y'):
    dot = graphviz.Digraph("LR Automata", graph_attr={'label':'LR Parser Automata', 'splines':'true'}, edge_attr={'arrowsize':'0.5'})
else:
    dot = graphviz.Digraph("LR Automata", graph_attr={'label':'LR Parser Automata'}, edge_attr={'arrowsize':'0.5'})

edge_labels = input("Do you want edge labels to be visible? (y/n) ")

parser_file_name = "y.output"


lines_words = []
with open(parser_file_name, "r") as file:
    data = file.readlines()
    for line in data:
        words = line.split()
        if(len(words) == 0):
            continue
        lines_words.append(words)

numberStates = 0
list_pairs = []
currState = -1
for word_list in lines_words:
    if(word_list[0] == 'State'):
        if(len(word_list) != 2):
            continue
        itemName = "I" + word_list[1]
        dot.node(itemName)
        currState += 1
    elif(currState != -1):
        if 'state' in word_list:
            list_pairs.append([currState, word_list[0], int(word_list[-1])])
        

for x in list_pairs:
    startName = "I" + str(x[0])
    endName = "I" + str(x[2])
    hoverLabel = x[1] + ": " + "I" + str(x[0]) + "->I" + str(x[2]) 
    if(edge_labels=='y'):
        dot.edge(endName, startName, x[1])
    else:
        dot.edge(startName, endName, tooltip=hoverLabel)


#print(list_pairs)


file_name = "graph.dot"
dot_file = open(file_name, 'w')
dot_file.write(dot.source)