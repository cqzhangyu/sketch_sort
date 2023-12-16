import heapq

def dijkstra(n,m,s,edges):
    graph = [[]for _ in range(n+1)]
    for u,v,c in edges:
        graph[u].append((v,c))
        graph[v].append((u,c))
        
    distances = [float('inf')]*(n+1)
    visited = [False]*(n+1)
    
    distances[s] = 0
    pq = [(0,s)]
    while pq:
        dist,u = heapq.heappop(pq)
        if visited[u]:
            continue
        visited[u] = True
        for v,weight in graph[u]:
            if dist + weight < distances[v]:
                distances[v] = dist+weight
                heapq.heappush(pq,(distances[v],v))
                
                distances = [-1 if d == float('inf')
            else d for d in distances]
    return distances[1:]


n,m,s = map(int,input().split())

edges = []
for _ in range(m):
    u,v,c = map(int,input().split())
    edges.append((u,v,c))

shortest_paths = dijkstra(n,m,s,edges)
print(shortest_paths)
