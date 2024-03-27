import { StatusEnum } from "types"
import webClient from "../../WebClient"

export function updateStatus(status: StatusEnum, description: string): void {
  webClient.updateStatus(status, description);
}
