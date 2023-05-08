import logging
import sys
import uuid

from bottle import Bottle, request, response, run
from cloudevents.sdk import converters
from cloudevents.sdk.event import v1 as event

app = Bottle()
app_logger = logging.getLogger('app_logger')
app_logger.setLevel(logging.INFO)

handler = logging.StreamHandler(sys.stdout)
# handler = logging.FileHandler('app.log')
app_logger.addHandler(handler)


def log_event():
    in_headers = dict(request.headers)
    in_data = request.body.getvalue()
    cloudevent = event.Event()
    cloudevent.SetEventType('com.example.event')
    cloudevent.SetEventTypeVersion('v1.0')
    cloudevent.SetSource('example.com/app')
    cloudevent.SetData(in_data)

    app_logger.info('Received CloudEvent')
    app_logger.debug(f'{str(cloudevent)}')


# @app.hook('before_request')
# def add_cloudevent_header():
#     event_id = request.headers.get('ce-id', None) or str(uuid.uuid4())
#     event_type = request.headers.get('ce-type', None)
#     event_source = request.headers.get('ce-source', None) or 'example.com/app'
#     event_data = request.body
#
#     cloudevent = event.Event()
#     cloudevent.SetID(event_id)
#     cloudevent.SetEventType(event_type)
#     cloudevent.SetSource(event_source)
#     cloudevent.SetData(event_data)
#
#     converter = converters.dump(cloudevent)
#     for header, value in converter['headers'].items():
#         response.headers[header] = str(value)


# @app.hook('after_request')
# def log_request_header():
#     log_event()


@app.route('/')
def index():
    obj = {
        'li': [3, 4, 5, 7],
        'str': 'hello',
        'range': range(30),
    }
    app_logger.info(f'obj id {hex(id(obj))}')
    return 'Hello, World!'


if __name__ == '__main__':
    run(app, host='0.0.0.0', port=5000)
