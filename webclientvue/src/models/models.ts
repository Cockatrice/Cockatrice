export interface Todo {
  id: number;
  content: string;
}

export interface Meta {
  totalCount: number;
}

export interface Games {
  age: string;
  description: string;
  creator: string;
  type: string;
  restrictions: string;
  players: string;
  spectators: string | number;
}

