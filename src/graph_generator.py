import graphviz
splines = input("Do you want splines in your graph? (y/n) ")

if(splines == 'y'):
    dot = graphviz.Digraph("LR Automata", graph_attr={'label':'LR Parser Automata', 'splines':'true'}, edge_attr={'arrowsize':'0.5'})
else:
    dot = graphviz.Digraph("LR Automata", graph_attr={'label':'LR Parser Automata'}, edge_attr={'arrowsize':'0.5'})

edge_labels = input("Do you want edge labels to be visible? (y/n) ")

parser_file_name = "y.output"


lines_words = []
terminals = []
Nonterminals = []

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
catchTerminals = 0
catchNonterminals = 0
for word_list in lines_words:
    if(word_list[0] == 'Terminals,'):
        catchTerminals = 1
    if(word_list[0] == 'Nonterminals,'):
        catchTerminals = 0
        catchNonterminals = 1

    if(word_list[0] == 'State'):
        if(len(word_list) != 2):
            continue
        if(word_list[1] == '0'):
            catchNonterminals = 0
        itemName = "I" + word_list[1]
        dot.node(itemName)
        currState += 1
    elif(currState != -1):
        if 'state' in word_list:
            list_pairs.append([currState, word_list[0], int(word_list[-1])])

    if(catchTerminals):
        terminals.append(word_list[0])

    if(catchNonterminals):
        Nonterminals.append(word_list[0])


for x in list_pairs:
    startName = "I" + str(x[0])
    endName = "I" + str(x[2])
    if x[1] in terminals:
        hoverLabel = "Terminal "
    elif x[1] in Nonterminals:
        hoverLabel = "Nonterminal "
    hoverLabel += x[1] + ": " + "I" + str(x[0]) + "->I" + str(x[2]) 
    if(edge_labels=='y'):
        dot.edge(startName, endName, x[1], tooltip=hoverLabel)
    else:
        dot.edge(startName, endName, tooltip=hoverLabel)


#print(list_pairs)


file_name = "graph.dot"
dot_file = open(file_name, 'w')
dot_file.write(dot.source)