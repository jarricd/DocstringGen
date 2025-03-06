import logging
import requests


def get_all_presets(self):
    logging.info("Getting all presets from GenieACS.")
    try:
        preset_get_request = requests.get(f"{self.url}/presets/")
        preset_get_request.raise_for_status()
    except (HTTPError, ConnectionError) as e:
        logging.error(f"Failed to get provisions from GenieACS due to {e}.")
        raise

    try:
        parsed_preset_json = preset_get_request.json()
    except JSONDecodeError as e:
        msg = f"Received JSON is incorrectly formatted. Received data: {preset_get_request.text}. Exception message: {e}."
        logging.error(msg)
        raise

    logging.info(
        f"Get preset request successful. Status code {preset_get_request.status_code}"
    )
    return parsed_preset_json


def test_2(x: int, y: int):
    z = x + y
    return z ** 2